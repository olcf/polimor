/****************************************************************************
 * Copyright 2023 UT Battelle, LLC
 *
 * This work was supported by the Oak Ridge Leadership Computing Facility at
 * the Oak Ridge National Laboratory, which is managed by UT Battelle, LLC for
 * the U.S. DOE (under the contract No. DE-AC05-00OR22725).
 *
 * This file is part of the PoliMOR project.
 ****************************************************************************/

#ifndef __MESSSAGE_JSON_PARSER_BOOST_IMPL_H__
#define __MESSSAGE_JSON_PARSER_BOOST_IMPL_H__


#include <boost/system/detail/errc.hpp>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <exception>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <memory>
#include <optional>
#include <stack>
#include <system_error>
#include <string_view>
#include <variant>
#include <vector>
#include <any>

#include <boost/system/errc.hpp>
#include <boost/json.hpp>
#include <boost/json/basic_parser_impl.hpp>
#include <boost/json/error.hpp>
#include <boost/system/system_error.hpp>
#include <boost/system/system_category.hpp>

#include "./serializers.h"
#include "../../common/qs_exception.h"

// /* Forward declaration */
// template<typename MSG, typename IMPL>
// class deserializer;

template<typename MSG>
class json_deserializer_impl {

    
    public:

        struct handler {

            /* Make sure to use boost here and not the std implementations. */
            using error_code = boost::system::error_code;
            using error = boost::json::error;
            using errc = boost::system::errc::errc_t;
            using source_location = boost::source_location;
            using string_view = boost::core::string_view;

            /* Declaration of types */
            using string_handler = std::function<void(string_view, MSG &)>;
            using integer_handler = std::function<void(uint64_t, MSG &)>;
            using floating_point_handler = std::function<void(double, MSG &)>;
            using bool_handler = std::function<void(bool, MSG &)>;
            using null_handler = std::function<void(MSG &)>;
            struct object_handler;
            struct array_handler;

            /* Variant for all the of the handler types */
            using value_handler = std::variant<string_handler, // 0
                                            integer_handler,  // 1
                                            floating_point_handler, // 2
                                            bool_handler,  // 3
                                            null_handler, // 4
                                            object_handler,  // 5
                                            array_handler>; // 6

       
            /* C++ blood magic here that lets me template multiple lambdas 
            for visiting variants. The second line is a deduction guide to 
            help the compilers out with the first rule */
            template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
            //template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

            
            struct object_handler {

                /* TODO might see if there is a more efficient way to store than
                * a vector, perhaps just an array */
                /* List of the handlers by key*/
                std::vector<std::pair<std::string, value_handler>> _handlers_by_key;


                /** 
                * @brief Constructor for the object handler that takes a list of 
                * key/handler pairs.  This will move the entries from the input
                * and to the class container. 
                */
                object_handler(std::vector<std::pair<std::string, value_handler>> &&handlers) :
                    _handlers_by_key(std::move(handlers)) { }
                
                
                /**
                * @brief Searches for the value handler that corresponds to the
                *        key and returns a const reference to the handler.  
                *        Otherwise it throw an invalid_argument exception.
                * 
                * @param key 
                * @return const value_handler&  Reference to the handle to use.
                * @throws invalid_argument If no matching key can be found.
                */
                const value_handler &operator()(const string_view &key) const {

                    /* Find the hander that corresponds to the key */
                    auto it = std::ranges::find(_handlers_by_key, key, 
                        [](const auto &e) noexcept -> const std::string & { 
                            return std::get<std::string>(e); 
                        });

                    /*  No key found */
                    if(it == _handlers_by_key.end()) {
                        throw std::invalid_argument(std::string("Handler not found for key ") + key.data());
                    }

                    /* Return a reference to the handler */
                    return std::get<1>(*it);
                } 
            };

            struct array_handler {

                /* TODO std::any is used here to avoid circular type dependency, 
                further optimization could be achieved by replacing std::any
                with just a storage buffer for handler and new placement */
                std::any _element_handler;

                // array_handler() = delete;
               // array_handler(const array_handler&) = delete;
                //array_handler &operator=(const array_handler &) = delete;

                /**
                * @brief Construct a new array handler object
                * 
                * @param handler Moves the handler to this value container. 
                */
                array_handler(value_handler handler) : 
                    _element_handler(std::move(handler)) {
                }

                /**
                * @brief Returns a reference to the handler to use for elements
                * in the array.
                * 
                * @return const value_handler& 
                */
                const value_handler &operator()() const {
                    return std::any_cast<const value_handler &>(_element_handler);
                };
            };


            /* Static for the class template that has the handlers to use */
            static const value_handler _starting_handler;
            
            /**
            * @brief Stack of references to the handlers to use.  The top of the
            * stack is the current handler in use for the value in the json element
            * being parsed by the primary handler.
            */
            std::stack<std::reference_wrapper<const value_handler>> _handler_stack;

            /* Stack for the array handlers to determine level of nesting */
            std::stack<std::reference_wrapper<const array_handler>> _array_stack;

            /* Reference to the message being created*/
            MSG *_msg = nullptr;

            /* Holds an exception that we don't want to pass through the 
             * C code or noexcept functions for performance reasons. */
            std::exception_ptr _eptr;

            /**
             * @brief Set the msg object to be filled in. 
             * 
             * @param msg 
             */
            void set_msg(MSG &msg) {
                _msg = std::addressof(msg);
            }

            void _reset() {
                
                while(not _handler_stack.empty()) {
                    _handler_stack.pop();
                }

                while(not _array_stack.empty()) {
                    _array_stack.pop();
                }
            }

            /** Boilerplate and required entries for boost json handler*/
            constexpr static std::size_t max_object_size = std::size_t(-1);
            constexpr static std::size_t max_array_size = std::size_t(-1);
            constexpr static std::size_t max_key_size = std::size_t(-1);
            constexpr static std::size_t max_string_size = std::size_t(-1);

            
            /**
             * @brief Called at the start of a json document.
             * 
             * @param ec Error code indicating the error type that occurred.
             * @return true Success.
             * @return false Error.
             */
            bool on_document_begin(error_code &ec) noexcept { 

                /* Return error if the stack is not empty */
                [[unlikely]]
                if(not _handler_stack.empty() or not _array_stack.empty()) {
                    BOOST_STATIC_CONSTEXPR source_location loc = BOOST_JSON_SOURCE_POS;
                    BOOST_JSON_ASSIGN_ERROR_CODE(ec, error::exception, &loc);
                    return false;
                }
                
                /* Turn off warying about starting handler not being in defined 
                 * in the header file. */
                #pragma clang diagnostic push
                #pragma clang diagnostic ignored "-Wundefined-var-template"

                /* Push the first handler onto the stack */
                _handler_stack.push(json_deserializer_impl<MSG>::handler::_starting_handler);

                #pragma clang diagnostic pop

                /* No error */
                return true; 
            }

            /**
             * @brief Called at the end of a json document
             * 
             * @param ec Error code indicating error.
             * @return true Success.
             * @return false Error occurred.
             */
            bool on_document_end(error_code &ec) noexcept { 
                
                /* Verifies that the stack is now empty */
                [[unlikely]]
                if(not _handler_stack.empty() or not _array_stack.empty()) {
                    BOOST_STATIC_CONSTEXPR source_location loc = BOOST_JSON_SOURCE_POS;
                    BOOST_JSON_ASSIGN_ERROR_CODE(ec, error::exception, &loc);
                }
      
                /* Success */
                return true; 
            }

            /**
             * @brief Handles the start of an object.  Two valid handlers when
             * called are the object_handler or the array_handler. 
             * The array_handler must load an object_handler for this object. 
             * 
             * @param ec Error code returned indicating the error that occurred.
             * @return true  Success.
             * @return false Failure, ec should be set to indicate the error.
             */
            bool on_object_begin(error_code &ec) noexcept {
                
                /* Handle the top of the stack by type */
                return std::visit(
                    overloaded{          

                        /* Confirm that there is an object handler on the stack.*/        
                        [&ec](const object_handler &handler) noexcept -> bool {
                            return true;
                        },

                        /* If there is an array handler when an object starts
                           then call the array handler, ensure that the next
                           handler for the array is indeed an object_handler,
                           and put that on the stack, otherwise error. */
                        [this, &ec](const array_handler &handler) noexcept -> bool {

                            /* Calls the array hander to get the next handler*/
                            auto h = handler();
                            
                            /* Check that the next handler is an object handler */
                            [[likely]]
                            if(std::holds_alternative<object_handler>(h)) {
    
                                _handler_stack.push(h);
                                return true;

                            /* Invalid handler*/
                            } else {
                                BOOST_STATIC_CONSTEXPR source_location loc = BOOST_JSON_SOURCE_POS;     
                                ec.assign(errc::invalid_argument, boost::system::system_category(), &loc);
                                
                                return false;
                            }
                        },

                        /* Invalid handler */
                        [&ec](auto handler) noexcept -> bool {
                            BOOST_STATIC_CONSTEXPR source_location loc = BOOST_JSON_SOURCE_POS;     
                            ec.assign(errc::invalid_argument, boost::system::system_category(), &loc);
                            
                            return false;
                        }
                    }, _handler_stack.top().get());
            }

            /**
             * @brief At the end of an object so pop the object_handler from
             * the stack.
             * 
             * @param ec Holds the error that occurred during the call.
             * @return true  On success.
             * @return false On failure.
             */
            bool on_object_end(std::size_t&, error_code &ec) noexcept { 
        
                /* At the end of the current object so pop its handler */
                _handler_stack.pop();

                return true; 
            }

            

            /**
             * @brief Called at start of an array.  An array handler must be
             * present.  
             * 
             * @param ec Error code that has the error that occurred during the 
             * call.
             * @return true On success. 
             * @return false On failure.
             */
            bool on_array_begin(error_code &ec) noexcept { 

                /* Handle the start of a new array with the top of the stack
                 * handler. */
                return std::visit(
                    overloaded{
                        
                        /* Confirm that there is an array handler on the stack */
                        /* TODO need to the handle the condition when there is 
                           an array of arrays*/
                        [this, &ec](const array_handler &handler) noexcept -> bool {

                            /* This array handler has been seen before so 
                             * we are encountering another array.  We need
                             * to invoke the current array_handler to get 
                             * the next array_handler */
                            [[unlikely]]
                            if(not _array_stack.empty() and 
                               (std::addressof(_array_stack.top().get()) == std::addressof(handler))) {
                                
                                const value_handler &new_array_handler = handler();

                                /* Not equal to nullptr when valid handler */
                                [[likely]]
                                if(std::holds_alternative<array_handler>(new_array_handler)) {

                                    _array_stack.push(std::get<array_handler>(new_array_handler));
                                    _handler_stack.push(new_array_handler);
                                        
                                /* Incorrect type */
                                } else {

                                    BOOST_STATIC_CONSTEXPR source_location loc = BOOST_JSON_SOURCE_POS;     
                                    ec.assign(errc::invalid_argument, boost::system::system_category(), &loc);
                        
                                    return false;
                                }

                            /* First time array handler seen */
                            } else {
                                _array_stack.push(handler);
                            }
                            
                            return true;
                        },

                        /* Invalid handler */
                        [&ec](auto handler) noexcept -> bool {
                            BOOST_STATIC_CONSTEXPR source_location loc = BOOST_JSON_SOURCE_POS;     
                            ec.assign(errc::invalid_argument, boost::system::system_category(), &loc);
                                
                            return false;
                        }

                    }, _handler_stack.top().get());
            }

            /**
             * @brief Called at the end of the current json array.  This 
             * function ensures that array handler is indeed being popped from 
             * the stack.
             * 
             * @param ec Error code that has the error type causing failure.
             * @return true  On success.
             * @return false On failure.
             */
            bool on_array_end(std::size_t&, error_code &ec) noexcept { 
            
                /* Get the current handler which should be the array handler */
                const auto &handler = _handler_stack.top();

                /* Verify that we have an array handler being popped. */
                return std::visit(

                    /* Just confirm that there is an array handler on the 
                     * stack. */
                    overloaded{
                        
                        /* Only accepted value */
                        [this](const array_handler &handler) noexcept -> bool {
                            
                            _array_stack.pop();
                             /* At the end of the current array so pop its handler */
                            _handler_stack.pop();

                            return true;
                        },

                        /* Invalid handler */
                        [&ec](auto handler) noexcept -> bool {
                            BOOST_STATIC_CONSTEXPR source_location loc = BOOST_JSON_SOURCE_POS;     
                            ec.assign(errc::invalid_argument, boost::system::system_category(), &loc);
                                
                            return false;
                        }

                    }, handler.get());
            }

            /**
             * @brief This function should never be called as we do not handle
             * partial values. 
             * 
             * @param ec 
             * @return false failure.
             */
            bool on_key_part(string_view, std::size_t, error_code &ec) noexcept {
            
                BOOST_STATIC_CONSTEXPR source_location loc = BOOST_JSON_SOURCE_POS;     
                ec.assign(errc::invalid_argument, boost::system::system_category(), &loc);
                                
                return false;
            }

            /**
             * @brief Called when a a key is encountered in an object.  It uses
             * the object handler on the top of the stack to find the 
             * handler for the value of the key.
             * 
             * @param key Name of the key.
             * @param size 
             * @param ec Holds the error code if an error happened.
             * @return true On success.
             * @return false On failure.
             */
            bool on_key(string_view key, std::size_t size, error_code &ec) noexcept {
                
                /* The only acceptable hander on the top of the stack is the
                 * object handler so we verify it, then use it to lookup the
                 * next handler by the key value and put that handler on the
                 * top of the stack. */
                return std::visit(
                    overloaded{
                        /* Only the object_keys_handler is valid in this context*/
                        [key, &ec, this](const object_handler &handler) noexcept -> bool {
                            
                            try {
                                _handler_stack.push(handler(key));
                                return true;
                    
                            } catch(std::exception &e) {
                                BOOST_STATIC_CONSTEXPR source_location loc = BOOST_JSON_SOURCE_POS;     
                                ec.assign(errc::invalid_argument, boost::system::system_category(), &loc);
                                return false;
                            }        
                        },

                        /* TODO Invalid handler */
                        [&ec](auto h) noexcept -> bool {
                            BOOST_STATIC_CONSTEXPR source_location loc = BOOST_JSON_SOURCE_POS;     
                            ec.assign(errc::invalid_argument, boost::system::system_category(), &loc);
                                
                            return false;
                        }

                    },  _handler_stack.top().get());
            }

            /**
             * @brief This function should never be called as we do not handle
             * partial values. 
             * 
             * @param ec 
             * @return false failure.
             */
            bool on_string_part(string_view, std::size_t, error_code &ec) noexcept { 
                
                BOOST_STATIC_CONSTEXPR source_location loc = BOOST_JSON_SOURCE_POS;     
                ec.assign(errc::invalid_argument, boost::system::system_category(), &loc);
                                
                return false;
            }

            /**
             * @brief Called when a string value is encountered.  It handles
             * the string if it either the value of key/value pair in an object
             * or an element in the array.
             * 
             * @param value String value to use as the value. 
             * @param ec Holds the error_code if an error happened.
             * @return true On success.
             * @return false On failure.
             */
            bool on_string(string_view value, std::size_t, error_code &ec) noexcept { 
                
                /* Handles a string value.  Either the string value is already
                 * on the stack (first case) from an object handler or
                 * we are in an array (second case) so call the array handler 
                 * to get the string handler. */
                return std::visit(
                    
                    overloaded {

                        /* Single value handler so execute it and then pop it from
                        * the stack. */
                        [value, &ec, this](const string_handler &handler) noexcept -> bool {
                            
                            try {
                                handler(value, *_msg);

                                 _handler_stack.pop();
                            
                                return true;

                            } catch (...) {
                                ec.assign(errc::invalid_argument, boost::system::system_category());
                                return false;
                            }
                        },

                        /* Array handler so call it the handle to get the value 
                        *  handler but do not pop it off  */
                        [value, &ec, this](const array_handler &handler) noexcept -> bool {
                        
                            /* Call the array handler to get the value handle and
                            * make sure that it is the correct type */
                            auto handle_ptr = std::get_if<string_handler>(
                                    std::addressof(handler()));

                            /* Not equal to nullptr when valid handler, call the
                            * value handler */
                            [[likely]]
                            if(handle_ptr) {
                                
                                try {
                                    (*handle_ptr)(value, *_msg);
                                    return true;
                                } catch (...) {
                                    ec.assign(errc::invalid_argument, boost::system::system_category());
                                    return false;
                                }

                            /* Incorrect type */
                            } else {

                                BOOST_STATIC_CONSTEXPR source_location loc = BOOST_JSON_SOURCE_POS;     
                                ec.assign(errc::invalid_argument, boost::system::system_category(), &loc);
                        
                                return false;
                            }
                        },

                        /* Invalid handler */
                        [&ec](auto h) noexcept -> bool {
                            
                            BOOST_STATIC_CONSTEXPR source_location loc = BOOST_JSON_SOURCE_POS;     
                            ec.assign(errc::invalid_argument, boost::system::system_category(), &loc);
                        
                            return false;
                        }, 

                    }, _handler_stack.top().get()); 
            }

            /**
             * @brief This function should never be called as we do not handle
             * partial values. 
             * 
             * @param ec 
             * @return false failure.
             */
            bool on_number_part(string_view value, error_code &ec) noexcept { 
                
                BOOST_STATIC_CONSTEXPR source_location loc = BOOST_JSON_SOURCE_POS;     
                ec.assign(errc::invalid_argument, boost::system::system_category(), &loc);
                                    
                return false;
            }

            /**
             * @brief Called when an int64_t value is encountered in the json 
             * string. This function processes the value based on wether the 
             * value was from an object key/value pair or part of an array.
             * 
             * @param value The value encountered by the parser.
             * @param sv 
             * @param ec Error code that has the error that occurred.
             * @return true On success.
             * @return false On failure.
             */
            bool on_int64(std::int64_t value, string_view sv, error_code &ec) noexcept { 
           
                /* Relies on the uint64 handler */
                return on_uint64(value, sv, ec); 
            }

            /**
             * @brief Called when an uint64_t value is encountered in the json 
             * string. This function processes the value based on wether the 
             * value was from an object key/value pair or part of an array.
             * 
             * @param value The value encountered by the parser.
             * @param sv 
             * @param ec Error code that has the error that occurred.
             * @return true On success.
             * @return false On failure.
             */
            bool on_uint64(std::uint64_t value, string_view, error_code &ec) { 
                   
                /* Processes the value based on which handler is at the top
                 * of the stack.  If a string handler is present then it
                 * just calls that with the value and pops the stack. For an
                 * array handler, it calls the the hander to get the integer
                 * handler for the value. All other handler types are invalid. */
                return std::visit(
                    
                    overloaded {

                        /* Single value handler so execute it and then pop it from
                        * the stack. */
                        [this, value, &ec](const integer_handler &handler) noexcept -> bool {
                            
                            try {
                                handler(value, *_msg);

                                 _handler_stack.pop();
                            
                                return true;

                            } catch (...) {
                                ec.assign(errc::invalid_argument, boost::system::system_category());
                                return false;
                            }
                        },

                        /* Array handler so call it the handle to get the value 
                         * handler but do not pop it off  */
                        [this, value, &ec](const array_handler &handler) noexcept -> bool {
                    
                            /* Call the array handler to get the value handle and
                            * make sure that it is the correct type */
                            auto handle_ptr = std::get_if<integer_handler>(
                                    std::addressof(handler()));

                            /* Not equal to nullptr when valid handler */
                            [[likely]]
                            if(handle_ptr) {

                                try {
                                    (*handle_ptr)(value, *_msg);

                                    return true;

                                } catch (...) {
                                    ec.assign(errc::invalid_argument, boost::system::system_category());
                                    return false;
                                }
                                
                            /* Incorrect type */
                            } else {

                                BOOST_STATIC_CONSTEXPR source_location loc = BOOST_JSON_SOURCE_POS;     
                                ec.assign(errc::invalid_argument, boost::system::system_category(), &loc);
                        
                                return false;
                            }
                        },

                        /* Invalid handler */
                        [&ec](auto h) noexcept -> bool {
                            
                            BOOST_STATIC_CONSTEXPR source_location loc = BOOST_JSON_SOURCE_POS;     
                            ec.assign(errc::invalid_argument, boost::system::system_category(), &loc);
                        
                            return false;
                        }

                    }, _handler_stack.top().get()); 
            }

            bool on_double(double value, string_view, error_code &ec) noexcept { 
                
                /* Processes the value based on which handler is at the top
                 * of the stack.  If a string handler is present then it
                 * just calls that with the value and pops the stack. For an
                 * array handler, it calls the the hander to get the integer
                 * handler for the value. All other handler types are invalid. */
                return std::visit(
                    
                    overloaded {

                        /* Single value handler so execute it and then pop it from
                        * the stack. */
                        [this, value, &ec](const floating_point_handler &handler) noexcept -> bool {
                            
                            try {
                                handler(value, *_msg);

                                 _handler_stack.pop();
                            
                                return true;

                            } catch (...) {
                                ec.assign(errc::invalid_argument, boost::system::system_category());
                                return false;
                            }
                        },

                        /* Array handler so call it the handle to get the value 
                         * handler but do not pop it off  */
                        [this, value, &ec](const array_handler &handler) noexcept -> bool {
                    
                            /* Call the array handler to get the value handle and
                            * make sure that it is the correct type */
                            auto handle_ptr = std::get_if<floating_point_handler>(
                                    std::addressof(handler()));

                            /* Not equal to nullptr when valid handler */
                            [[likely]]
                            if(handle_ptr) {

                                try {
                                    (*handle_ptr)(value, *_msg);

                                    return true;

                                } catch (...) {
                                    ec.assign(errc::invalid_argument, boost::system::system_category());
                                    return false;
                                }
                                
                            /* Incorrect type */
                            } else {

                                BOOST_STATIC_CONSTEXPR source_location loc = BOOST_JSON_SOURCE_POS;     
                                ec.assign(errc::invalid_argument, boost::system::system_category(), &loc);
                        
                                return false;
                            }
                        },

                        /* Invalid handler */
                        [&ec](auto h) noexcept -> bool {
                            
                            BOOST_STATIC_CONSTEXPR source_location loc = BOOST_JSON_SOURCE_POS;     
                            ec.assign(errc::invalid_argument, boost::system::system_category(), &loc);
                        
                            return false;
                        }

                    }, _handler_stack.top().get()); 
            }

            bool on_bool(bool value, error_code &ec) noexcept { 
                
                /* Processes the value based on which handler is at the top
                 * of the stack.  If a string handler is present then it
                 * just calls that with the value and pops the stack. For an
                 * array handler, it calls the the hander to get the integer
                 * handler for the value. All other handler types are invalid. */
                return std::visit(
                    
                    overloaded {

                        /* Single value handler so execute it and then pop it from
                        * the stack. */
                        [this, value, &ec](const bool_handler &handler) noexcept -> bool {
                            
                            try {
                                handler(value, *_msg);

                                 _handler_stack.pop();
                            
                                return true;

                            } catch (...) {
                                ec.assign(errc::invalid_argument, boost::system::system_category());
                                return false;
                            }
                        },

                        /* Array handler so call it the handle to get the value 
                         * handler but do not pop it off  */
                        [this, value, &ec](const array_handler &handler) noexcept -> bool {
                    
                            /* Call the array handler to get the value handle and
                            * make sure that it is the correct type */
                            auto handle_ptr = std::get_if<bool_handler>(
                                    std::addressof(handler()));

                            /* Not equal to nullptr when valid handler */
                            [[likely]]
                            if(handle_ptr) {

                                try {
                                    (*handle_ptr)(value, *_msg);

                                    return true;

                                } catch (...) {
                                    ec.assign(errc::invalid_argument, boost::system::system_category());
                                    return false;
                                }
                                
                            /* Incorrect type */
                            } else {

                                BOOST_STATIC_CONSTEXPR source_location loc = BOOST_JSON_SOURCE_POS;     
                                ec.assign(errc::invalid_argument, boost::system::system_category(), &loc);
                        
                                return false;
                            }
                        },

                        /* Invalid handler */
                        [&ec](auto h) noexcept -> bool {
                            
                            BOOST_STATIC_CONSTEXPR source_location loc = BOOST_JSON_SOURCE_POS;     
                            ec.assign(errc::invalid_argument, boost::system::system_category(), &loc);
                        
                            return false;
                        }

                    }, _handler_stack.top().get()); 
            }

            bool on_null(error_code &ec) noexcept { 
                
                /* Processes the value based on which handler is at the top
                 * of the stack.  If a string handler is present then it
                 * just calls that with the value and pops the stack. For an
                 * array handler, it calls the the hander to get the integer
                 * handler for the value. All other handler types are invalid. */
                return std::visit(
                    
                    overloaded {

                        /* Single value handler so execute it and then pop it from
                         * the stack. */
                        [this, &ec](const null_handler &handler) noexcept -> bool {
                            
                            try {
                                handler(*_msg);

                                 _handler_stack.pop();
                            
                                return true;

                            } catch (...) {
                                ec.assign(errc::invalid_argument, boost::system::system_category());
                                return false;
                            }
                        },

                        /* Array handler so call it the handle to get the value 
                         * handler but do not pop it off  */
                        [this, &ec](const array_handler &handler) noexcept -> bool {
                    
                            /* Call the array handler to get the value handle and
                            * make sure that it is the correct type */
                            auto handle_ptr = std::get_if<null_handler>(
                                    std::addressof(handler()));

                            /* Not equal to nullptr when valid handler */
                            [[likely]]
                            if(handle_ptr) {

                                try {
                                    (*handle_ptr)(*_msg);

                                    return true;

                                } catch (...) {
                                    ec.assign(errc::invalid_argument, boost::system::system_category());
                                    return false;
                                }
                                
                            /* Incorrect type */
                            } else {

                                BOOST_STATIC_CONSTEXPR source_location loc = BOOST_JSON_SOURCE_POS;     
                                ec.assign(errc::invalid_argument, boost::system::system_category(), &loc);
                        
                                return false;
                            }
                        },

                        /* Invalid handler */
                        [&ec](auto h) noexcept -> bool {
                            
                            BOOST_STATIC_CONSTEXPR source_location loc = BOOST_JSON_SOURCE_POS;     
                            ec.assign(errc::invalid_argument, boost::system::system_category(), &loc);
                        
                            return false;
                        }

                    }, _handler_stack.top().get()); 
            }

            /**
             * @brief This function should never be called as we do not handle
             * comments.
             * 
             * @param ec 
             * @return false failure.
             */
            bool on_comment_part(string_view, error_code &ec) noexcept {
                BOOST_STATIC_CONSTEXPR source_location loc = BOOST_JSON_SOURCE_POS;     
                ec.assign(errc::invalid_argument, boost::system::system_category(), &loc);
                                    
                return false;
            }

            /**
             * @brief This function should never be called as we do not handle
             * comments.
             * 
             * @param ec 
             * @return false failure.
             */
            bool on_comment(string_view, error_code &ec) noexcept { 
                BOOST_STATIC_CONSTEXPR source_location loc = BOOST_JSON_SOURCE_POS;     
                ec.assign(errc::invalid_argument, boost::system::system_category(), &loc);
                                    
                return false;
             }
        };

    /* Provide the parser with our handler implementation. */
    boost::json::basic_parser<handler> _p; 

    private:
        void _validate_message(const MSG &msg) const;
        
        void _reset() {
            _p.handler()._reset();
            _p.reset();    
        }



    public:

        /**
         * @brief Constructor.
         * 
         */
        json_deserializer_impl() : _p(boost::json::parse_options()) { };

        json_deserializer_impl(const json_deserializer_impl &) = delete;
        json_deserializer_impl &operator=(const json_deserializer_impl &) = delete;

        json_deserializer_impl(json_deserializer_impl &&o) = delete; 
        json_deserializer_impl &operator=(json_deserializer_impl &&rhs) = delete;

        /** Takes something convertible to a string_view and parses it
         * tyring to return the MSG type. */
        MSG operator()(std::string_view buffer) {

            try {

                MSG msg;
                boost::system::error_code ec;
                

                _p.handler().set_msg(msg);

                /* Parse the data */
                _p.write_some(false, buffer.data(), buffer.length(), ec);   

                /* Error occurred */
                if(ec) {
                    _reset();

                    /* TODO throw an exception */
                    throw qs_exception("Parsing error: "+ec.message()+" at " + ec.location().to_string());
                }

                /* Validate the message */
                _validate_message(msg); 
                    
                /* NVRO allows us to "return" without penalty */
                return msg;

            } catch(...) {
                 std::throw_with_nested(qs_exception("Unable to deserialize message"));
            }
        } 
};




#endif // __MESSSAGE_JSON_PARSER_BOOST_IMPL_H__


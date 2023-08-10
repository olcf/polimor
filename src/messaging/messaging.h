/****************************************************************************
 * Copyright 2023 UT Battelle, LLC
 *
 * This work was supported by the Oak Ridge Leadership Computing Facility at
 * the Oak Ridge National Laboratory, which is managed by UT Battelle, LLC for
 * the U.S. DOE (under the contract No. DE-AC05-00OR22725).
 *
 * This file is part of the PoliMOR project.
 ****************************************************************************/

#ifndef __MESSAGING_H__
#define __MESSAGING_H__

#include <cinttypes>
#include <concepts>
#include <string_view>
#include <memory>
#include <algorithm>
#include <tuple>
#include <type_traits>
#include <utility>

#include "../common/pimpl.h"

#include "./details/messages.h"
#include "./details/serializers.h"
#include "./details/messaging_common.h"
#include "./details/message_json_deserializer_boost_impl.h"
#include "./details/message_json_serializer_boost_impl.h"
#include "./details/posix_messaging_impl.h"
#include "./details/jetstream_messaging_impl.h"




/* Wrapper class for message queue writer */
class message_queue_publisher {

    friend class messaging_service;
    
    private:

        template<MsgPublisher ...types>
        using message_queue_publisher_impl = std::variant<types...>;

        qs::common::shared_pimpl<
            message_queue_publisher_impl<
                posix_message_queue_publisher_impl, 
                jetstream_message_queue_publisher_impl>> _pimpl;

        message_queue_publisher(const MsgPublisher auto &impl) 
            requires (not std::same_as<typeof(impl), message_queue_publisher>): 
            _pimpl(impl) { }        

        message_queue_publisher &operator=(const MsgPublisher auto &impl) 
            requires (not std::same_as<typeof(impl), message_queue_publisher>){
            _pimpl = impl; 
            return *this;
        }
        
        message_queue_publisher(MsgPublisher auto &&impl) 
            requires (not std::same_as<typeof(impl), message_queue_publisher>) :
            _pimpl(std::move(impl)) { }
        
        message_queue_publisher &operator=(MsgPublisher auto &&impl) 
            requires (not std::same_as<typeof(impl), message_queue_publisher>) {
            _pimpl = std::move(impl);
            return *this;
        }

    public:
        message_queue_publisher() = delete; 
        
        message_queue_publisher(const message_queue_publisher &o) : _pimpl(o._pimpl) {}
        message_queue_publisher &operator=(const message_queue_publisher &rhs) {
            _pimpl = rhs._pimpl;
            return *this;
        }

        message_queue_publisher(message_queue_publisher &&o) : _pimpl(std::move(o._pimpl)) {}
        message_queue_publisher &operator=(message_queue_publisher &&rhs) {
            _pimpl = std::move(rhs._pimpl);
            return *this;
        }

        template<typename MSG, 
                 template<typename> typename SERIALIZER=json_serializer_impl> 
            requires MsgSerializerLike<SERIALIZER, MSG>
        void send(const MSG &msg) {
            std::visit([&msg](auto &&impl) { 
                    impl.template send<MSG, SERIALIZER>(msg); 
                }, *_pimpl);
        };
};



/* Wrapper class for message queue reader */
class message_queue_subscriber {

    friend class messaging_service;

    private:

        /* Restrict the variant types that the variant can hold to 
         * implementations that support MsgSubscriber */
        template<MsgSubscriber ...types>
        using message_queue_subscriber_impl = std::variant<types...>;
        
        /* Shared ponter to the impl */
        qs::common::shared_pimpl<
            message_queue_subscriber_impl<
                posix_message_queue_subscriber_impl,                        
                jetstream_message_queue_subscriber_impl>> _pimpl;
       
        message_queue_subscriber(const MsgSubscriber auto &impl) 
            requires (not std::same_as<typeof(impl), message_queue_subscriber>) : 
            _pimpl(impl) {}
        message_queue_subscriber &operator=(const MsgSubscriber auto &impl) 
            requires (not std::same_as<typeof(impl), message_queue_subscriber>) {

            _pimpl = impl;
            return *this;
        }

        message_queue_subscriber(MsgSubscriber auto &&impl) 
            requires (not std::same_as<typeof(impl), message_queue_subscriber>) : 
            _pimpl(std::move(impl)) {}
        message_queue_subscriber &operator=(MsgSubscriber auto &&impl) 
            requires (not std::same_as<typeof(impl), message_queue_subscriber>) {
            _pimpl = std::move(impl);
            return *this;
        }

    public:
        message_queue_subscriber() = delete; 


        message_queue_subscriber(const message_queue_subscriber &o) : _pimpl(o._pimpl) {}
        message_queue_subscriber &operator=(const message_queue_subscriber &rhs) {
            _pimpl = rhs._pimpl;
            return *this;
        }

        message_queue_subscriber(message_queue_subscriber &&o) : _pimpl(std::move(o._pimpl)) {}
        message_queue_subscriber &operator=(message_queue_subscriber &&rhs) {
            _pimpl = std::move(rhs._pimpl);
            return *this;
        }

 

        template<typename MSG, 
                 template<typename> typename DESERIALIZER=json_deserializer_impl>
            requires MsgDeserializerLike<DESERIALIZER, MSG> &&  
                     std::default_initializable<MSG>
        MSG receive() {
            return std::visit([](auto &&impl) { 
                    return impl.template receive<MSG, DESERIALIZER>(); 
                }, *(this->_pimpl));
        }
};


/* Wrapper class for messaging_service service */
class messaging_service {
    
    template<messaging_services, typename... args> 
    friend messaging_service create_messaging_service(args...);


    private:
    
        /* Restrict the types the variant can hold, must be MsgService 
         * compatiable */
        template<MsgService ...types>
        using messaging_service_impl = std::variant<types...>;

        /* Shared pointer to the variant with the possible types */
        qs::common::shared_pimpl<
            messaging_service_impl<posix_messaging_service_impl, 
                                   jetstream_messaging_service_impl>> _pimpl;


        messaging_service(const MsgService auto &impl) 
            requires (not std::same_as<typeof(impl), messaging_service>) : 
            _pimpl(impl) {}

        messaging_service &operator=(const MsgService auto &impl) 
            requires (not std::same_as<typeof(impl), messaging_service>) {
            _pimpl = impl;
            return *this;
        }

        messaging_service(MsgService auto &&impl) 
            requires (not std::same_as<typeof(impl), messaging_service>) : 
            _pimpl(std::move(impl)) {}
        messaging_service &operator=(MsgService auto &&impl) 
            requires (not std::same_as<typeof(impl), messaging_service>) {
            _pimpl = std::move(impl);
            return *this;
        }

       
    public:

        messaging_service() = delete; 


        messaging_service(const messaging_service &o) : _pimpl(o._pimpl) {}
        messaging_service &operator=(const messaging_service &rhs) {
            _pimpl = rhs._pimpl;
            return *this;
        }

        messaging_service(messaging_service &&o) : _pimpl(std::move(o._pimpl)) {}

        messaging_service &operator=(messaging_service &&rhs) {
            _pimpl = std::move(rhs._pimpl);
            return *this;
        }

        messaging_services get_type() { 
            return std::visit([](auto &&impl) { 
                return impl.get_type();
            }, *(this->_pimpl));  
        }
        
        template<typename... Args>
        message_queue_publisher create_queue_publisher(Args... args) { 

            return std::visit(
                [l = { args... }](auto &&impl) {
                    return message_queue_publisher(std::move(impl.create_queue_publisher(l)));
                }, *(this->_pimpl));
        };
            
        template<typename... Args>
        message_queue_subscriber create_queue_subscriber(Args... args) { 
            return std::visit(
                [l = { args... }](auto &&impl) { 
                    return message_queue_subscriber(std::move(impl.create_queue_subscriber(l)));
                }, *(this->_pimpl));
        };
};



/* Template function for creating actual service. Must define an instance
   implementation for each type*/
template<messaging_services, typename... args> 
messaging_service create_messaging_service(args...);



#endif // __MESSAGING_H__v
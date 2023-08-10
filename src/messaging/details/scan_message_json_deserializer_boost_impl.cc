/****************************************************************************
 * Copyright 2023 UT Battelle, LLC
 *
 * This work was supported by the Oak Ridge Leadership Computing Facility at
 * the Oak Ridge National Laboratory, which is managed by UT Battelle, LLC for
 * the U.S. DOE (under the contract No. DE-AC05-00OR22725).
 *
 * This file is part of the PoliMOR project.
 ****************************************************************************/

#include <exception>
#include <iostream>
#include <system_error>
#include <tuple>
#include <utility>
#include <variant>
#include <vector>

#include <boost/json.hpp>
#include <boost/json/basic_parser_impl.hpp>

#include "./serializers.h"

#include "./message_json_deserializer_boost_impl.h"
#include "../../common/qs_exception.h"



using error_code = boost::system::error_code;
using string_view = boost::core::string_view;


/* Type alias from the class */
using string_handler = json_deserializer_impl<scan_message>::handler::string_handler;
using integer_handler = json_deserializer_impl<scan_message>::handler::integer_handler;
using floating_point_handler = json_deserializer_impl<scan_message>::handler::floating_point_handler;
using bool_handler = json_deserializer_impl<scan_message>::handler::bool_handler;
using null_handler = json_deserializer_impl<scan_message>::handler::null_handler;
using object_handler = json_deserializer_impl<scan_message>::handler::object_handler;
using array_handler = json_deserializer_impl<scan_message>::handler::array_handler;
using value_handler = json_deserializer_impl<scan_message>::handler::value_handler;


template<>
void json_deserializer_impl<scan_message>::_validate_message(const scan_message &msg) const { 
    
    std::chrono::time_point<std::chrono::system_clock> epoch;

    if(msg.atime == epoch) {
        throw qs_exception("Error deserializing scan message: atime is invalid");
        
    } else if(msg.mtime == epoch) {
        throw qs_exception("Error deserializing scan message: mtime is invalid");

    } else if(msg.type != 'f' and msg.type != 'd') {
        throw qs_exception("Error deserializing scan message: type is invalid");

    } else if(msg.filesys.empty()) {
        throw qs_exception("Error deserializing scan message: filesys is invalid");

    } else if(msg.path.empty()) { 
        throw qs_exception("Error deserializing scan message: path is invalid");

    } else if(msg.fid.empty()) {
        throw qs_exception("Error deserializing scan message: fid is invalid");

    }
}

static object_handler _scan_message_format_object_handler({ 
    {
        std::string("filesys"),
        value_handler([](string_view value, scan_message &msg) noexcept {
            msg.filesys = value;
        })
    }, 
    {
        std::string("ost_pool"),
        value_handler([](string_view value, scan_message &msg) noexcept {
            msg.ost_pool = value;
        })
    },
    { 
        std::string("stripe_count"),
        value_handler(
            std::in_place_type<integer_handler>,
            [](std::uint64_t value, scan_message &msg) noexcept {
                msg.stripe_count = value; 
            })
    },
    { 
        std::string("fid"),
        value_handler([](string_view value, scan_message &msg) noexcept {
            msg.fid = string_view(value);
        })
    }       
});


object_handler _scan_message_object_handler = { {

    {
        std::string("type"), 
        value_handler([](string_view value, scan_message &msg) {
            
            if(value != "d" and value != "f") {
                throw std::invalid_argument(std::string("Invalid file type: ")+
                                            value.data());
            }

            msg.type = value[0];
        }),
    },
    {
        std::string("path"), 
        value_handler([](string_view value, scan_message &msg) noexcept {

            /* TODO could add a check here for proper filesystem path format */
            msg.path = value;
        }),
    },  
    {    
        std::string("atime"),
        value_handler(
            std::in_place_type<integer_handler>,
            [](int64_t value, scan_message &msg) noexcept {
                msg.atime = std::chrono::system_clock::from_time_t(value);  
            }),
    },
    {
        std::string("mtime"), 
        value_handler(
            std::in_place_type<integer_handler>,
            [](uint64_t value, scan_message &msg) noexcept {
                msg.mtime = std::chrono::system_clock::from_time_t(value);
            }),
    },
    {
        std::string("size"), 
        value_handler(
            std::in_place_type<integer_handler>,
            [](uint64_t value, scan_message &msg) noexcept {
                msg.size = value; 
            }),
    },
    {
        std::string("uid"),
        value_handler(
            std::in_place_type<integer_handler>,
            [](uint64_t value, scan_message &msg) noexcept {
                msg.uid = value;
            })
    },
    {   
        std::string("gid"),
        value_handler(
            std::in_place_type<integer_handler>,
            [](uint64_t value, scan_message &msg) noexcept {
            msg.gid = value;
        })
    },
    {
        std::string("format"),
        value_handler(std::move(_scan_message_format_object_handler))          
    }
} };


template<>
const value_handler json_deserializer_impl<scan_message>::handler::_starting_handler(
    std::move(_scan_message_object_handler));
    
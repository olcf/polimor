/****************************************************************************
 * Copyright 2023 UT Battelle, LLC
 *
 * This work was supported by the Oak Ridge Leadership Computing Facility at
 * the Oak Ridge National Laboratory, which is managed by UT Battelle, LLC for
 * the U.S. DOE (under the contract No. DE-AC05-00OR22725).
 *
 * This file is part of the PoliMOR project.
 ****************************************************************************/

#include <iostream>
#include <tuple>
#include <utility>
#include <variant>
#include <vector>

#include <boost/json.hpp>
#include <boost/json/basic_parser_impl.hpp>

#include "./serializers.h"
#include "../../common/qs_exception.h"
#include "./message_json_deserializer_boost_impl.h"



using error_code = boost::system::error_code;
using string_view = boost::core::string_view;


/* Type alias from the class */
using string_handler = json_deserializer_impl<migration_message>::handler::string_handler;
using integer_handler = json_deserializer_impl<migration_message>::handler::integer_handler;
using floating_point_handler = json_deserializer_impl<migration_message>::handler::floating_point_handler;
using bool_handler = json_deserializer_impl<migration_message>::handler::bool_handler;
using null_handler = json_deserializer_impl<migration_message>::handler::null_handler;
using object_handler = json_deserializer_impl<migration_message>::handler::object_handler;
using array_handler = json_deserializer_impl<migration_message>::handler::array_handler;
using value_handler = json_deserializer_impl<migration_message>::handler::value_handler;



template<>
void json_deserializer_impl<migration_message>::_validate_message(const migration_message &msg) const{ 
    
    if(msg.path.empty()) {
        throw qs_exception("Error deserializing migration message: path is invalid");
    }
}

object_handler _migration_message_object_handler = { {
    
    {
        std::string("path"), 
        value_handler([](string_view value, migration_message &msg) noexcept {

            /* TODO could add a check here for proper filesystem path format */
            msg.path = value;
        }),
    },  
    
} };


template<>
const value_handler json_deserializer_impl<migration_message>::handler::_starting_handler(
    std::move(_migration_message_object_handler));
    
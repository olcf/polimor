/****************************************************************************
 * Copyright 2023 UT Battelle, LLC
 *
 * This work was supported by the Oak Ridge Leadership Computing Facility at
 * the Oak Ridge National Laboratory, which is managed by UT Battelle, LLC for
 * the U.S. DOE (under the contract No. DE-AC05-00OR22725).
 *
 * This file is part of the PoliMOR project.
 ****************************************************************************/


#include <string_view>
#include <boost/json.hpp>


#include "../messages.h"

/*
void json_parser_boost_impl::parse(std::string_view data, 
                                   scan_message<json_parser_boost_impl> &msg) {

}
*/

template<typename MSG>
MSG json_deserializer(std::string_view buffer);

template<>
scan_message json_deserializer(std::string_view buffer) {


    return {};
}



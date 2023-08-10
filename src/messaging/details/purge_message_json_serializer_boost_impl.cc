/****************************************************************************
 * Copyright 2023 UT Battelle, LLC
 *
 * This work was supported by the Oak Ridge Leadership Computing Facility at
 * the Oak Ridge National Laboratory, which is managed by UT Battelle, LLC for
 * the U.S. DOE (under the contract No. DE-AC05-00OR22725).
 *
 * This file is part of the PoliMOR project.
 ****************************************************************************/


#include <string>
#include <string_view>
#include <cstdio>
#include <sstream>
#include <system_error>
#include <cerrno>

#include "./messages.h"
#include "./message_json_serializer_boost_impl.h"

template<>
std::string_view 
json_serializer_impl<purge_message>::operator()(
        const purge_message &msg, 
        const std::string_view buffer) const {

    /* Write the data to a buffer */
    int rc = snprintf(const_cast<char *>(buffer.data()),
                      buffer.size(), 
                      "{ "
                        "\"path\": \"%s\" "
                      "}",
                      msg.path.c_str());

    /* Error serializing the message */
    if(rc < 0) {
        throw std::system_error(errno, 
                                std::generic_category(), 
                          "Error serializing message");

    /* Buffer was too small */
    } else if(rc >= buffer.size()) {
        throw std::system_error(ENOMEM, 
                                std::generic_category(), 
                          "Output buffer too small");
        
    }
    
    /* Return the filled buffer */
    return { buffer.data(), static_cast<std::string_view::size_type>(rc + 1)};
}



template<>
std::string 
json_serializer_impl<purge_message>::operator()(const purge_message& msg) const {

  std::stringstream buffer;

  buffer << "{ \"path\": \"" << msg.path << "\" }";
  
  return buffer.str();
}
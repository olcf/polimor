/****************************************************************************
 * Copyright 2023 UT Battelle, LLC
 *
 * This work was supported by the Oak Ridge Leadership Computing Facility at
 * the Oak Ridge National Laboratory, which is managed by UT Battelle, LLC for
 * the U.S. DOE (under the contract No. DE-AC05-00OR22725).
 *
 * This file is part of the PoliMOR project.
 ****************************************************************************/

#include <chrono>
#include <string>
#include <sstream>
#include <string_view>
#include <cstdio>
#include <cinttypes>
#include <system_error>
#include <cerrno>

#include "./messages.h"
#include "./message_json_serializer_boost_impl.h"

template<>
std::string_view 
json_serializer_impl<scan_message>::operator()(
        const scan_message &msg, 
        const std::string_view buffer) const {

    /* Write the data to a buffer */
    int rc = snprintf(const_cast<char *>(buffer.data()),
                      buffer.size(), 
                      "{ "
                        "\"type\": \"%c\", "
                        "\"path\": \"%s\", "
                        "\"atime\": %" PRIu64 ", "
                        "\"mtime\": %" PRIu64 ", "
                        "\"size\": %" PRIu64 ", "
                        "\"uid\": %" PRIu64 ", "
                        "\"gid\": %" PRIu64 ", "
                        "\"format\": { " 
                            "\"filesys\": \"%s\", "
                            "\"ost_pool\": \"%s\", " 
                            "\"stripe_count\": %" PRIu64 ", "
                            "\"fid\": \"%s\" "
                         "}" 
                      "}",
                      msg.type, 
                      msg.path.c_str(), 
                      std::chrono::system_clock::to_time_t(msg.atime),
                      std::chrono::system_clock::to_time_t(msg.mtime),
                      msg.size, 
                      msg.uid, 
                      msg.gid, 
                      msg.filesys.c_str(), 
                      msg.ost_pool.c_str(), 
                      msg.stripe_count, 
                      msg.fid.c_str());

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
json_serializer_impl<scan_message>::operator()(const scan_message &msg) const {

    std::stringstream buffer;
    
    buffer << "{ "
              "\"type\": \"" << msg.type << "\", "
              "\"path\": \"" << msg.path << "\", "
              "\"atime\": " << std::chrono::system_clock::to_time_t(msg.atime) << ", "
              "\"mtime\": " << std::chrono::system_clock::to_time_t(msg.mtime) << ", "
              "\"size\": " << msg.size << ", "
              "\"uid\": " << msg.uid << ", "
              "\"gid\": " << msg.gid << ", "
              "\"format\": { " 
                    "\"filesys\": \"" << msg.filesys << "\", "
                    "\"ost_pool\": \"" << msg.ost_pool << "\", "
                    "\"stripe_count\": " << msg.stripe_count << ", "
                    "\"fid\": \"" << msg.fid << "\" "
                    "}" 
              "}";
                     
                      
    return buffer.str();
}
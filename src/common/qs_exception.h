/****************************************************************************
 * Copyright 2023 UT Battelle, LLC
 *
 * This work was supported by the Oak Ridge Leadership Computing Facility at
 * the Oak Ridge National Laboratory, which is managed by UT Battelle, LLC for
 * the U.S. DOE (under the contract No. DE-AC05-00OR22725).
 *
 * This file is part of the PoliMOR project.
 ****************************************************************************/

#pragma once

#include <source_location>
#include <stdexcept>


class qs_exception : public std::runtime_error {

    private: 
        std::source_location _location;

    public:
        qs_exception() : std::runtime_error("Unknown error"),  
                        _location(std::source_location::current()) { }

        qs_exception(const std::string &msg) : std::runtime_error(msg),
                                        _location(std::source_location::current()) { }

        const std::source_location &location() const { return _location; }
};

// void throw_exception(const std::string_view msg, 
//                      const std::source_location location = 
//                                     std::source_location::current()) {

    
//     throw std::runtime_error(msg, location);
// }
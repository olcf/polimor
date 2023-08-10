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


#include "config_parser.h"

std::string configfile = "/workspace/policy-engine/src/tests/resources/config.yaml";

int main(int argc, const char* argv[]) {

  
    using qs::common::ConfigType;

    auto config_parser = qs::common::parse_config<ConfigType::YAML>(configfile);
    
    std::clog << config_parser.get_version() << std::endl;

    std::clog << config_parser.get_messaging_service_backend() << std::endl;

    for(const auto &server : config_parser.get_messaging_service_nats_servers()) {
        std::clog << server << std::endl;
    }

    for(const auto &queue_name : config_parser.get_messaging_service_nats_queue_names()) {
        std::clog << queue_name << std::endl;
    }


    for(const auto &property : config_parser.get_messaging_service_nats_queue_properties_by_name("scan")) {
        std::clog << property.first << ": " << property.second << std::endl;
    }


    for(const auto &agent_type: config_parser.get_agent_types()) {
        std::clog << agent_type << std::endl;
    }

    for(const auto &agent_properties : config_parser.get_agent_properties_by_id("purge_agent0")) {
        std::clog << agent_properties.first << ": " << agent_properties.second << std::endl;
    }

    return EXIT_SUCCESS;
}
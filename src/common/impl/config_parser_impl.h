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

#include <iostream>
#include <string>
#include <vector>

#include "yaml-cpp/yaml.h"

namespace qs::common::impl {

    class yaml_config_parser {

        private:
            YAML::Node root_node;

        public:
            yaml_config_parser() = default;
            yaml_config_parser(const yaml_config_parser&) = default;
            yaml_config_parser(yaml_config_parser&&) = default;
            yaml_config_parser& operator=(const yaml_config_parser&) = default;
            yaml_config_parser& operator=(yaml_config_parser&&) = default;

            yaml_config_parser(YAML::Node &&root_node) : 
                root_node(std::move(root_node)) {
            }


            std::string get_version() const;
            std::string get_messaging_service_backend() const;

            std::vector<std::string> get_messaging_service_nats_servers() const;
            std::vector<std::string> 
            get_messaging_service_nats_queue_names() const;

            std::map<std::string, std::string> 
            get_messaging_service_nats_queue_properties_by_name(
                std::string_view queue_name) const;

            std::vector<std::string> get_agent_types() const;
            
            std::map<std::string, std::string> 
            get_agent_properties_by_id(std::string_view id) const;
    };


    yaml_config_parser yaml_parse_config(std::string_view filename);
}
/****************************************************************************
 * Copyright 2023 UT Battelle, LLC
 *
 * This work was supported by the Oak Ridge Leadership Computing Facility at
 * the Oak Ridge National Laboratory, which is managed by UT Battelle, LLC for
 * the U.S. DOE (under the contract No. DE-AC05-00OR22725).
 *
 * This file is part of the PoliMOR project.
 ****************************************************************************/


#include <list>
#include <sstream>
#include <string_view>
#include <string>
#include <stdexcept>


char schema[] = {
    #include "schema.inc"
};



#include "yaml-cpp/yaml.h"
#include "valijson/adapters/yaml_cpp_adapter.hpp"
#include "valijson/utils/yaml_cpp_utils.hpp"
#include "valijson/schema.hpp"
#include <valijson/schema_parser.hpp>
#include <valijson/validation_results.hpp>
#include <valijson/validator.hpp>


#include "./config_parser_impl.h"


namespace qs::common::impl {

   yaml_config_parser yaml_parse_config(std::string_view filename) {

        /* Read the schema*/
        YAML::Node schemadoc;

        try {
            
            schemadoc = YAML::Load(schema);

        } catch (const YAML::ParserException& e) {
            throw std::runtime_error(std::string("Error reading schema for config: ")+e.what());
        }

        valijson::Schema schema;
        valijson::SchemaParser parser;

        /* Wrap the YAML::Node holding the schema with the valijson adapter */
        valijson::adapters::YamlCppAdapter schema_adapter(schemadoc);
        
        try{
            /* Populate the valijson schema using the yaml cpp adapter */
            parser.populateSchema(schema_adapter, schema);
        
        }catch(const std::exception& e){
            throw std::runtime_error(std::string("Error reading schema for config: ")+e.what());
        }
       
           
        YAML::Node configdoc;
    
        /* Read the config file */
        auto retval = valijson::utils::loadDocument(filename.data(), configdoc);
    
        [[unlikely]]
        if(not retval) {
            throw std::runtime_error(std::string("Failed read the config file: ")+std::string(filename));
        }


        /* Values types are okay if they are convertible to the desired type */
        valijson::Validator validator(valijson::Validator::kWeakTypes);

        /* Create the adapter for config file */
        valijson::adapters::YamlCppAdapter config_adapter(configdoc);

        /* Validate the config file */
        valijson::ValidationResults results;
        retval = validator.validate(schema, config_adapter, &results);

        [[unlikely]]
        if(not retval) {

            valijson::ValidationResults::Error error;
            std::stringstream msg;

            msg << "Error validating the config file: " << filename << std::endl;
           
            for(unsigned int errnum = 1; results.popError(error); ++errnum) {

                msg << "Error num: " << errnum << std::endl;
                msg << "\tContext: " << std::endl;

                for(auto const& line : error.context) {
                    msg << line << "\n\t\t";
                }

                msg << "\n\tDescription: " << error.description << std::endl;
            }

            throw std::runtime_error(msg.str());
        }


        return { std::move(configdoc) };
    }


    
    std::string yaml_config_parser::get_version() const {
        return root_node["version"].as<std::string>();
    }

    std::string yaml_config_parser::get_messaging_service_backend() const {
        return root_node["messaging_service"]["backend"].as<std::string>();
    }

    std::vector<std::string> yaml_config_parser::get_messaging_service_nats_servers() const {

        /* Verify*/
        if(root_node["messaging_service"]["backend"].as<std::string>() != "nats") {
            throw std::runtime_error("Config error: not the NATS backend");
        }

        /* Get the top level array server node */
        auto node = root_node["messaging_service"]["config"]["servers"];
        
        /* Build an array of the servers*/
        std::vector<std::string> servers;
        
        for(const auto &server : node) {
            std::clog << server << std::endl;
            servers.emplace_back(server["host"].as<std::string>() +
                                    ":" + 
                                    server["port"].as<std::string>());
        }


        return std::move(servers);
    }

    std::vector<std::string> 
    yaml_config_parser::get_messaging_service_nats_queue_names() const {

        /* Verify*/
        if(root_node["messaging_service"]["backend"].as<std::string>() != "nats") {
            throw std::runtime_error("Config error: not the NATS backend");
        }

        /* Get the top level array queue node */
        auto node = root_node["messaging_service"]["config"]["queues"];
        
        /* Build an array of the servers*/
        std::vector<std::string> queues;
        
        for(const auto &queue : node) {
            queues.emplace_back(queue.first.as<std::string>());
        }

        return std::move(queues);
    }

    std::map<std::string, std::string> 
    yaml_config_parser::get_messaging_service_nats_queue_properties_by_name(
        std::string_view queue_name) const {
        
        /* Verify*/
        if(root_node["messaging_service"]["backend"].as<std::string>() != "nats") {
            throw std::runtime_error("Config error: not the NATS backend");
        }

        /* Get the top level array queue node */
        auto node = root_node["messaging_service"]["config"]["queues"][queue_name.data()];
        
        if(not node) {
            throw std::runtime_error(std::string("Config error: ") + 
                                        queue_name.data() + 
                                        " queue not found");
        }

        /* Build a map of the properties */
        auto properties = std::map<std::string, std::string>();

        for(const auto &property : node) {
            properties.emplace(property.first.as<std::string>(), 
                                property.second.as<std::string>());
        }
        

        return std::move(properties);
    }

    std::vector<std::string> yaml_config_parser::get_agent_types() const {

        std::vector<std::string> agent_types;

        for(const auto &agent : root_node["agents"]) {
            agent_types.emplace_back(agent.first.as<std::string>());
        }

        return std::move(agent_types);
    }


    std::map<std::string, std::string> 
    yaml_config_parser::get_agent_properties_by_id(std::string_view id) const {

        for(const auto &agent_types : root_node["agents"]) {

            for(const auto &agent_config : agent_types.second) {
                
                /* Entry found */
                if(agent_config["id"].as<std::string>() == id) {

                    /* Build a map of the properties */
                    auto properties = std::map<std::string, std::string>();

                    for(const auto &property : agent_config) {
                        properties.emplace(property.first.as<std::string>(), 
                                            property.second.as<std::string>());
                    }

                    properties.emplace("type", agent_types.first.as<std::string>());

                    return std::move(properties);
                }
            }
        }

        throw std::runtime_error(std::string("Config error: no agent with id: ") + id.data());
    }
}
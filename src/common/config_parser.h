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

#include <string>
#include <string_view>
#include <variant>
#include <concepts>
#include <vector>


#include "pimpl.h"

#include "impl/config_parser_impl.h"

namespace qs::common {

    enum class ConfigType { 
        YAML
    };


    template<typename ConfigParserImpl>
    concept ConfigParser = requires(ConfigParserImpl impl) {

        { impl.get_version() } -> std::same_as<std::string>;
        { impl.get_messaging_service_backend() } -> std::same_as<std::string>;
        { impl.get_messaging_service_nats_servers() } -> std::same_as<std::vector<std::string>>;
        { impl.get_messaging_service_nats_queue_names() } -> std::same_as<std::vector<std::string>>;
        { impl.get_messaging_service_nats_queue_properties_by_name(
                std::string_view{}) } -> std::same_as<std::map<std::string, std::string>>;
        { impl.get_agent_types() } -> std::same_as<std::vector<std::string>>;
        { impl.get_agent_properties_by_id(std::string_view{}) } -> std::same_as<std::map<std::string, std::string>>;
    };

    class config {

        /* Only the parse_config functions can call the constructor. */
        template<ConfigType type>
        friend config parse_config(std::string_view filename);

        private:

           
            /* Restrict the variant to ones that meet the ConfigParser concept \
             */
            template<ConfigParser ...types>
            using config_parser_impl = std::variant<types...>;
            
            /* Shared pointer to the underlying impls */
            qs::common::shared_pimpl<config_parser_impl<impl::yaml_config_parser>> _pimpl;
            
            config(const ConfigParser auto &impl1) 
                requires (not std::same_as<typeof(impl1), config>) : 
                _pimpl(std::move(impl1)) {
                
            }

        public:

            config() = delete;
            config(const config&) = delete;
            config& operator=(const config&) = delete;
            config(config&&) = delete;
            config& operator=(config&&) = delete;

            /**
             * @brief Get the version of the config used.
             * 
             * @return std::string holding the version of the config.
             */
            std::string get_version() const {
                return std::visit([](auto &&impl) -> std::string {
                    return impl.get_version();
                }, *_pimpl);
            }

            /**
             * @brief Get the messaging service backend.
             * 
             * @return std::string holding the messaging service backend.
             */
            std::string get_messaging_service_backend() const {
                return std::visit([](auto &&impl) -> std::string { 
                    return impl.get_messaging_service_backend();
                }, *_pimpl);
            }

            /**
             * @brief Get the messaging service nats servers object
             * 
             * @throws Runtime exception if there is an error.
             * @return std::list<std::string> 
             */
            std::vector<std::string> get_messaging_service_nats_servers() const {
                return std::visit([](auto &&impl) -> std::vector<std::string> {
                    return impl.get_messaging_service_nats_servers();
                }, *_pimpl);
            }

            /**
             * @brief Get the messaging service nats queue names 
             * 
             * @throws Runtime exception if there is an error.
             * @return std::vector<std::string> 
             */
            std::vector<std::string> get_messaging_service_nats_queue_names() const {
                return std::visit([](auto &&impl) -> std::vector<std::string> {
                    return impl.get_messaging_service_nats_queue_names();
                }, *_pimpl);
            }

            /**
             * @brief Gets a map of the queue properties by name.
             * 
             * @throws Runtime exception if there is an error.
             * @return std::map<std::string, std::string>
             */
            std::map<std::string, std::string> 
            get_messaging_service_nats_queue_properties_by_name(
                std::string_view queue_name) const {

                return std::visit([queue_name](auto &&impl) -> std::map<std::string, std::string> {
                    return impl.get_messaging_service_nats_queue_properties_by_name(queue_name);
                }, *_pimpl);
            }

            /**
             * @brief Get the agent types listed in the config file.
             * 
             * @return std::vector<std::string> of agent types.
             */
            std::vector<std::string> get_agent_types() const {
                return std::visit([](auto &&impl) -> std::vector<std::string> {
                    return impl.get_agent_types();
                }, *_pimpl);
            }

            /**
             * @brief Get the agent properties by the id of the agent.
             * 
             * @param id 
             * @return std::map<std::string, std::string> 
             */
            std::map<std::string, std::string> 
            get_agent_properties_by_id(std::string_view id) const {
                return std::visit([id](auto &&impl) -> std::map<std::string, std::string> {
                    return impl.get_agent_properties_by_id(id);
                }, *_pimpl);
            }
    };

    template<ConfigType type>
    config parse_config(std::string_view filename);


    template<>
    config parse_config<ConfigType::YAML>(std::string_view filename) {
        return config { impl::yaml_parse_config(filename) };
    }
}
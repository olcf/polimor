/****************************************************************************
 * Copyright 2023 UT Battelle, LLC
 *
 * This work was supported by the Oak Ridge Leadership Computing Facility at
 * the Oak Ridge National Laboratory, which is managed by UT Battelle, LLC for
 * the U.S. DOE (under the contract No. DE-AC05-00OR22725).
 *
 * This file is part of the PoliMOR project.
 ****************************************************************************/

#include <cstdlib>
#include <iostream>
#include <string_view>
#include <numeric>

#include <boost/program_options.hpp>

#include "../common/config_parser.h"
#include "../messaging/messaging.h"
#include "./recording_agents.h"


// constexpr std::string_view recorder_queue_name = "recorder";
// constexpr std::string_view recorder_consumer = "recorder-files-consumer";
// constexpr std::string_view recorder_subject = "recorder.files";


struct args {
    
    std::string id;

    std::string nats_url;
    std::string recorder_stream;
    std::string recorder_consumer;
    std::string recorder_subject;
};


struct args parse_config_file(std::string_view config_file, 
                              std::string_view agent_id) {

    using qs::common::parse_config; 
    using qs::common::ConfigType;

    try {

        /* Parse the yaml config file */
        auto config = parse_config<ConfigType::YAML>(config_file);

        
        /* Get the properties of the agent */
        auto properties = config.get_agent_properties_by_id(agent_id);

        /* Check that the id is a scan agent */
        if(properties.at("type") != "recorder_agents") {   
            std::cerr << "Invalid agent type: " << properties.at("type") << std::endl;
            exit(EXIT_FAILURE);
        }

        /* Get the messaging backend type and be sure that it is nats */
        const auto &msg_backend = config.get_messaging_service_backend();
        
        if(msg_backend != "nats") {
            std::cerr << "Unsupported messaging service backend: " << msg_backend << std::endl;
            exit(EXIT_FAILURE);
        }

        /* Check that the queue exists in the config */
        const auto &queue_names = config.get_messaging_service_nats_queue_names();

        const auto &queue = properties.at("queue");

        auto result = std::ranges::find(queue_names, queue);

        if(result == std::ranges::end(queue_names)) {
            std::cerr << "Invalid messaging queue: " << queue << std::endl;
            std::exit(EXIT_FAILURE);
        }

        /* Get the queue properties */
        const auto &queue_properties = 
            config.get_messaging_service_nats_queue_properties_by_name(queue);

        /* Build the nats url */
        const auto &nats_servers = config.get_messaging_service_nats_servers();

        auto nats_url = std::accumulate(std::next(nats_servers.begin()), 
                                                    nats_servers.end(), 
                                                            *nats_servers.begin(), 
                                                            [](const std::string &a, const std::string &b) { 
                                                                return a + "," + b; });

        /* Return the arg */
        return {
            .id                = std::move(properties.at("id")),
            .nats_url          = std::move(nats_url),
            .recorder_stream   = std::move(queue_properties.at("stream_name")),
            .recorder_consumer = std::move(queue_properties.at("consumer_name")),
            .recorder_subject  = std::move(queue_properties.at("subject"))
        };

    } catch(const std::out_of_range &e) {
        std::cerr << "Invalid config file: missing property" << std::endl;
        exit(EXIT_FAILURE);
    
    } catch(const std::exception &e) {
        std::cerr << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }
}


/**
 * @brief Parse the command line and return the processed arguments.
 * 
 * @param argc 
 * @param argv 
 * @return struct args 
 */
static struct args parse_commandline(int argc, char *argv[]) {

    namespace po = boost::program_options;

    struct args args;

    /* Build the description of the command line */
    po::options_description desc("Options");
    desc.add_options()
        ("help,h", "This help message")
        ("config", po::value<std::string>(), "Configuration file")
        ("id", po::value<std::string>(), "Id of this scan agent, must be present in the config file")
        ("nats_server,n", po::value<std::vector<std::string>>(), "URL to the NATS server (may be repeated), e.g. rage1.ccs.ornl.gov:4222")
        ("stream", po::value<std::string>(), "Nats name of the scan stream")
        ("consumer", po::value<std::string>(), "Nats name of the scan consumer")
        ("subject", po::value<std::string>(), "Nats scan subject");
        
    /* Progress the command line */
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm); 

    if (vm.count("help")) {
        std::cerr << desc << std::endl;
        exit(EXIT_SUCCESS);
    }

    /* Verify that id was given */
    if(vm.count("id") != 1) {
        std::cerr << "Id of this scan agent must be provided." << std::endl;
        exit(EXIT_FAILURE);

    /* Store the arg */
    } else {
        args.id = vm["id"].as<std::string>();
    }

    if(vm.count("nats_server") < 1) {
        std::cerr << "Error, must specify at least one NATS server." << std::endl;
        exit(EXIT_FAILURE);
    }

    /* Config was provided so parse it.  */
    if(vm.count("config") == 1) {
        args = std::move(parse_config_file(vm["config"].as<std::string>(), 
                                                        vm["id"].as<std::string>()));
    }


    /* Verify that nats servers where specified either in a config file or a 
     * via command line */
    if(vm.count("nats_server") < 1 && vm.count("config") != 1) {
        std::cerr << "Error, must specify at least one NATS server." << std::endl;
        exit(EXIT_FAILURE);

    /* If nats servers are specified on command line, then use them even if
     * they were specified in the config. */
    } else if(vm.count("nats_server") > 0) {

        const auto &nats_servers = vm["nats_server"].as<std::vector<std::string>>();

        /* Join the nats servers into one NATS url string */
        args.nats_url = std::accumulate(std::next(nats_servers.begin()), 
                                          nats_servers.end(), 
                                          *nats_servers.begin(), 
                                          [](const std::string &a, const std::string &b) { 
                                              return a + "," + b; 
                                          });
    }

    /* Return an arg struct of the arguments to the process */
    return std::move(args);
}

int main(int argc, char *argv[]) {

    auto args = parse_commandline(argc, argv);

    MsgService auto ms = 
        create_messaging_service<messaging_services::JETSTREAM>(
            std::string_view(args.nats_url));

    MsgSubscriber auto mq_sub = ms.create_queue_subscriber(
        std::string_view(args.recorder_stream), 
        std::string_view(args.recorder_consumer), 
        std::string_view(args.recorder_subject));

    recording_agent *agent = 
            create_recording_agent<RECORDING_AGENTS::SQLITE>(ms, mq_sub);

    agent->run();

    /* Return successfully */
    return EXIT_SUCCESS;
}
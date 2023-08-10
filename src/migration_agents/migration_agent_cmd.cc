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

#include "../messaging/messaging.h"
#include "../common/config_parser.h"
#include "../messaging/details/jetstream_messaging_impl.h"
#include "migration_agent.h"


// constexpr std::string_view migration_stream = "migration";
// constexpr std::string_view migration_consumer = "migration-files-consumer";
// constexpr std::string_view migration_subject = "migration.files.request";

struct args {

    std::string id;
    
    std::string nats_url;
    std::string migration_stream;
    std::string migration_consumer;
    std::string migration_subject;  

    bool dry_run;
};


/**
 * @brief Parses the config file and returns a struct of the args
 *        for this agent.
 * 
 * @param config_file 
 * @param agent_id 
 * @return struct args 
 */
struct args parse_config_file(std::string_view config_file, 
                              std::string_view agent_id) { 

    using qs::common::parse_config; 
    using qs::common::ConfigType;

    try {

        /* Parse the yaml config file */
        auto config = parse_config<ConfigType::YAML>(config_file);

        /* Get the properties of the agent */
        auto properties = config.get_agent_properties_by_id(agent_id);

        /* Check that the id is a migration agent */
        if(properties.at("type") != "migration_agents") {   
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
        const auto &queue_properties = config.get_messaging_service_nats_queue_properties_by_name(queue);

        /* Build the nats url */
        const auto &nats_servers = config.get_messaging_service_nats_servers();

        auto nats_url = std::accumulate(std::next(nats_servers.begin()), 
                                                        nats_servers.end(), 
                                                        *nats_servers.begin(), 
                                                        [](const std::string &a, const std::string &b) { 
                                                            return a + "," + b; });

        /* Return the arg struct with the values found in the config file. */ 
        return {  
            .id                 = std::move(properties.at("id")),
            .nats_url           = std::move(nats_url),
            .migration_stream   = std::move(queue_properties.at("stream_name")),
            .migration_consumer = std::move(queue_properties.at("consumer_name")),
            .migration_subject  = std::move(queue_properties.at("subject"))
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
        ("nats_server", po::value<std::vector<std::string>>(), "URL to the NATS server (may be repeated), e.g. rage1.ccs.ornl.gov:4222")
        ("stream", po::value<std::string>(), "Nats name of the scan stream")
        ("consumer", po::value<std::string>(), "Nats name of the scan consumer")
        ("subject", po::value<std::string>(), "Nats scan subject")
        ("dry-run,d", "Dry run, don't migrate anything");


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

    /* Verify that a stream was given either on command line or in a 
     * config file */
    if(vm.count("stream") != 1 && vm.count("config") < 1) {
        std::cerr << "Error, must specify a migration stream" << std::endl;
        exit(EXIT_FAILURE);

    /* If specified on the command line use that instead of the config file */
    } else if(vm.count("stream") == 1) {
        args.migration_stream = vm["stream"].as<std::string>();
    }

    /* Verify that a consumer was given either on command line or in a 
     * config file */
    if(vm.count("consumer")!= 1 && vm.count("config") < 1) {
        std::cerr << "Error, must specify a migration consumer" << std::endl;
        exit(EXIT_FAILURE);
    
    /* If specified on the command line use that instead of the config file */
    } else if(vm.count("consumer") == 1) {
        args.migration_consumer = vm["consumer"].as<std::string>();
    }

    /* Verify that a subject was given either on command line or in a 
     * config file */
    if(vm.count("subject") != 1 && vm.count("config") < 1) {
        std::cerr << "Error, must specify a migration subject" << std::endl;
        exit(EXIT_FAILURE);
    
    /* If specified on the command line use that instead of the config file */
    } else if(vm.count("subject") == 1) {
        args.migration_subject = vm["subject"].as<std::string>();
    }

    /* Dry run or not*/
    args.dry_run = (vm.count("dry-run")) ? true : false;
    
    /* Return an arg struct of the arguments to the process */
    return std::move(args);
}


int main(int argc, char* argv[]) {

    auto args = parse_commandline(argc, argv);

   /* Print the configuration */
    std::clog << "Migration agent id: " << args.id << std::endl;
    std::clog << "NATS server url: " << args.nats_url << std::endl;
    std::clog << "Migration stream: " << args.migration_stream << std::endl;
    std::clog << "Migration consumer: " << args.migration_consumer << std::endl;
    std::clog << "Migration subject: " << args.migration_subject << std::endl;

    std::clog << "Starting migration agent..." << std::endl;
    
    MsgService auto ms = 
        create_messaging_service<messaging_services::JETSTREAM>(
            std::string_view(args.nats_url));

    MsgSubscriber auto mq_sub = 
        ms.create_queue_subscriber(
            std::string_view(args.migration_stream), 
            std::string_view(args.migration_consumer), 
            std::string_view(args.migration_subject));

    migration_agent agent = create_migration_agent<migration_agents::LFS_MIGRATE>(mq_sub, args.dry_run);

    agent.run();

    return EXIT_SUCCESS;
}
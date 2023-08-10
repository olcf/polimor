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
#include <initializer_list>
#include <string_view>
#include <iostream>
#include <numeric>
#include <cstddef>

#include <boost/program_options.hpp>

#include <mqueue.h>
#include <tuple>

#include "../common/config_parser.h"
#include "../messaging/messaging.h"
#include "../messaging/details/jetstream_messaging_impl.h"
#include "policy_engine.h"


// constexpr std::string_view scan_stream   = "scan";
// constexpr std::string_view scan_consumer = "scan-files-consumer";
// constexpr std::string_view scan_subject  = "scan.files.results";

// constexpr std::string_view removal_stream = "purge";
// constexpr std::string_view removal_consumer = "purge-files-consumer";
// constexpr std::string_view removal_subject = "purge.files.request";

// constexpr std::string_view migration_stream = "migration";
// constexpr std::string_view migration_consumer = "migration-files-consumer";
// constexpr std::string_view migration_subject = "migration.files.request";

// constexpr std::string_view recorder_stream = "recorder";
// constexpr std::string_view recorder_consumer = "recorder-files-consumer";
// constexpr std::string_view recorder_subject = "recorder.files";



struct args {
    std::string id;
    std::string nats_url;

    std::string scan_stream   = "scan";
    std::string scan_consumer = "scan-files-consumer";
    std::string scan_subject  = "scan.files.results";

    std::string purge_stream = "purge";
    std::string purge_consumer = "purge-files-consumer";
    std::string purge_subject = "purge.files.request";

    std::string migration_stream = "migration";
    std::string migration_consumer = "migration-files-consumer";
    std::string migration_subject = "migration.files.request";

    std::string recorder_stream = "recorder";
    std::string recorder_consumer = "recorder-files-consumer";
    std::string recorder_subject = "recorder.files";
};


/**
 * @brief Parse the config file and create the args object based
 *        on the config file.
 * 
 * @param config_file The path to the config file.
 * @param agent_id The id of the agent.
 * @return struct args 
 */
static struct args parse_config_file(std::string_view config_file, 
                                     std::string_view agent_id) {

    using qs::common::parse_config; 
    using qs::common::ConfigType;

    try {
        
        /* Parse the yaml config file  */
        auto config = parse_config<ConfigType::YAML>(config_file);

        /* Get the properties for the agent */
        auto properties = config.get_agent_properties_by_id(agent_id);

        /* Check that the id is a scan agent */
        if(properties.at("type") != "policy_agents") {   
            std::cerr << "Invalid agent type: " << properties.at("type") << std::endl;
            exit(EXIT_FAILURE);
        }

        /* Get the messaging backend type and be sure that it is nats */
        const auto &msg_backend = config.get_messaging_service_backend();
        
        if(msg_backend != "nats") {
            std::cerr << "Unsupported messaging service backend: " << msg_backend << std::endl;
            exit(EXIT_FAILURE);
        }

        /* Check that the scan queue exists in the config */
        const auto &queue_names = config.get_messaging_service_nats_queue_names();

        auto &queue = properties.at("scan_queue");

        auto result = std::ranges::find(queue_names, queue);

        if(result == std::ranges::end(queue_names)) {
            std::cerr << "Invalid messaging queue for scan_queue: " << queue << std::endl;
            std::exit(EXIT_FAILURE);
        }

        /* Get the queue properties */
        const auto &scan_queue_properties = config.get_messaging_service_nats_queue_properties_by_name(queue);


        /* Check that the purge queue exists in the config */
        queue = properties.at("purge_queue");

        result = std::ranges::find(queue_names, queue);

        if(result == std::ranges::end(queue_names)) {
            std::cerr << "Invalid messaging queue for purge_queue: " << queue << std::endl;
            std::exit(EXIT_FAILURE);
        }

        /* Get the queue properties */
        const auto &purge_queue_properties = config.get_messaging_service_nats_queue_properties_by_name(queue);

        /* Check that the migration queue exists in the config */
        queue = properties.at("migration_queue");
        
        result = std::ranges::find(queue_names, queue);

        if(result == std::ranges::end(queue_names)) {
            std::cerr << "Invalid messaging queue for migration_queue: " << queue << std::endl;
            std::exit(EXIT_FAILURE);
        }

        /* Get the queue properties */
        const auto &migration_queue_properties = config.get_messaging_service_nats_queue_properties_by_name(queue);


        /* Build the nats url */
        const auto &nats_servers = config.get_messaging_service_nats_servers();

        auto nats_url = std::accumulate(std::next(nats_servers.begin()), 
                                                        nats_servers.end(), 
                                                        *nats_servers.begin(), 
                                                        [](const std::string &a, const std::string &b) { 
                                                        return a + "," + b; });
        /* Return the args */
        return {
            .id = std::move(properties.at("id")),
            .nats_url = std::move(nats_url),
            .scan_stream = std::move(scan_queue_properties.at("stream_name")),
            .scan_consumer = std::move(scan_queue_properties.at("consumer_name")),
            .scan_subject = std::move(scan_queue_properties.at("subject")),
            .purge_stream = std::move(purge_queue_properties.at("stream_name")),
            .purge_consumer = std::move(purge_queue_properties.at("consumer_name")),
            .purge_subject = std::move(purge_queue_properties.at("subject")),
            .migration_stream = std::move(migration_queue_properties.at("stream_name")),
            .migration_consumer = std::move(migration_queue_properties.at("consumer_name")),
            .migration_subject = std::move(migration_queue_properties.at("subject"))
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
        ("scan_stream", po::value<std::string>(), "Nats name of the scan stream")
        ("scan_consumer", po::value<std::string>(), "Nats name of the scan consumer")
        ("scan_subject", po::value<std::string>(), "Nats scan subject")
        ("purge_stream", po::value<std::string>(), "Nats name of the purge stream")
        ("purge_consumer", po::value<std::string>(), "Nats name of the purge consumer")
        ("purge_subject", po::value<std::string>(), "Nats purge subject")
        ("migration_stream", po::value<std::string>(), "Nats name of the migration stream")
        ("migration_consumer", po::value<std::string>(), "Nats name of the migration consumer")
        ("migration_subject", po::value<std::string>(), "Nats migration subject");

    /* Progress the command line */
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm); 

    if (vm.count("help")) {
        std::cerr << desc << std::endl;
        exit(EXIT_SUCCESS);
    }

    /* Verify that id was given */
    if(vm.count("id") < 1) {
        std::cerr << "Id of this scan agent must be provided." << std::endl;
        exit(EXIT_FAILURE);

    /* Store the arg */
    } else {
        args.id = vm["id"].as<std::string>();
    }


    /* Config file provided so parse it and use it to initialize args. */
    if(vm.count("config") > 0) {
        args = std::move(parse_config_file(vm["config"].as<std::string>(), 
                                        vm["id"].as<std::string>()));
    }

    /* Verify that nats servers were specified ieither in a config file or 
     * on command line. */
    if(vm.count("nats_server") > 0 and vm.count("config") < 1) {
        std::cerr << "Error, must specify at least one NATS server." << std::endl;
        exit(EXIT_FAILURE);

    /* If the the nats serverw are specified on command line, then use them 
     * potentially overriding values in the config file. */
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

    /* For each of these options check if they were give on the command line */
    for(const auto &option : 
        std::initializer_list<std::pair<std::string, std::string *>>{
            {"scan_stream", &args.scan_stream},
            {"scan_consumer", &args.scan_consumer}, 
            {"scan_subject", &args.scan_subject}, 
            {"purge_stream", &args.purge_stream}, 
            {"purge_consumer", &args.purge_consumer}, 
            {"purge_subject", &args.purge_subject}, 
            {"migration_stream", &args.migration_stream}, 
            {"migration_consumer", &args.migration_consumer}, 
            {"migration_subject", &args.migration_subject}}) {

        if(vm.count(option.first) == 1) {
            *option.second = vm[option.first].as<std::string>();
        }

        /* Check that the option was specified in either the config file or
         * on command line. */
        if(option.second->empty()) {
            std::cerr << "Error " + option.first + " must be provided." << std::endl;
        }
    }

    /* Return an arg struct of the arguments to the process */
    return std::move(args);
}




int main(int argc, char* argv[]) {


    auto args = parse_commandline(argc, argv);

    std::clog << "id: " << args.id << std::endl;
    std::clog << "nats_url: " << args.nats_url << std::endl;
    std::clog << "scan_stream: " << args.scan_stream << std::endl;
    std::clog << "scan_consumer: " << args.scan_consumer << std::endl;
    std::clog << "scan_subject: " << args.scan_subject << std::endl;
    std::clog << "purge_stream: " << args.purge_stream << std::endl;
    std::clog << "purge_consumer: " << args.purge_consumer << std::endl;
    std::clog << "purge_subject: " << args.purge_subject << std::endl;
    std::clog << "migration_stream: " << args.migration_stream << std::endl;
    std::clog << "migration_consumer: " << args.migration_consumer << std::endl;
    std::clog << "migration_subject: " << args.migration_subject << std::endl;
    std::clog << "recorder_stream: " << args.recorder_stream << std::endl;
    std::clog << "recorder_consumer: " << args.recorder_consumer << std::endl;
    std::clog << "recorder_subject: " << args.recorder_subject << std::endl;

    std::clog << "Starting policy agent..." << std::endl;
    
    MsgService auto ms = create_messaging_service<messaging_services::JETSTREAM>(
        std::string_view(args.nats_url));

    MsgSubscriber auto scan_mq_sub = 
        ms.create_queue_subscriber(
            std::string_view(args.scan_stream), 
            std::string_view(args.scan_consumer), 
            std::string_view(args.scan_subject));
    MsgPublisher auto removal_mq_pub = 
        ms.create_queue_publisher(
            std::string_view(args.purge_stream), 
            std::string_view(args.purge_consumer), 
            std::string_view(args.purge_subject));

    MsgPublisher auto migration_mq_pub = 
        ms.create_queue_publisher(
            std::string_view(args.migration_stream), 
            std::string_view(args.migration_consumer), 
            std::string_view(args.migration_subject));
    
    MsgPublisher auto recorder_mq_pub = 
        ms.create_queue_publisher(
            std::string_view(args.recorder_stream), 
            std::string_view(args.recorder_consumer), 
            std::string_view(args.recorder_subject));

    policy_engine *policy_engine = create_policy_engine(scan_mq_sub, 
                                                        removal_mq_pub, 
                                                        migration_mq_pub, 
                                                        recorder_mq_pub);


    policy_engine->run();


    return EXIT_SUCCESS;
}
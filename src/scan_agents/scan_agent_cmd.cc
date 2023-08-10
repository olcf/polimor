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
#include <vector>
#include <chrono>
#include <regex>
#include <ranges>
#include <numeric>

#include <mqueue.h>


#include <boost/program_options.hpp>

#include "../scan_agents/scan_agent.h"
#include "../common/config_parser.h"
#include "../messaging/messaging.h"
#include "../messaging/details/jetstream_messaging_impl.h"

// constexpr std::string_view scan_stream   = "scan";
// constexpr std::string_view scan_consumer = "scan-files-consumer";
// constexpr std::string_view scan_subject  = "scan.files.results";


struct args {
    
    std::string id;
    std::string directory;
    std::chrono::seconds scan_interval;

    std::string nats_url;
    std::string scan_stream;
    std::string scan_consumer;
    std::string scan_subject;
};



static std::chrono::seconds parse_interval(const std::string &spec) {

    std::chrono::seconds interval(0);

    /* Pattern */
    std::regex pattern("(?:(\\d+)d)?(?:(\\d+)h)?(?:(\\d+)m)?(?:(\\d+)s)?");
    std::smatch match;

    /* Attempt to match the interval spec */
    if(std::regex_match(spec, match, pattern) == false) {

        std::cerr << "Invalid interval specification: not of the proper format" << spec << std::endl;
        exit(EXIT_FAILURE);
    }


    /* Days was specified */
    if(match.length(1) > 0) {
        interval += std::chrono::days(std::stoi(match[1]));
    }
    
    /* Hours was specified */
    if(match.length(2) > 0) {

        auto hrs = std::chrono::hours(std::stoi(match[2]));

        if(match.length(1) > 0 && hrs >= std::chrono::days(1)) {
            std::cerr << "Invalid interval specification: hours exceeds " 
                      << std::chrono::duration_cast<std::chrono::hours>(std::chrono::days(1)).count() - 1 
                      << " when days also specified." 
                      << std::endl;
            exit(EXIT_FAILURE);
        }

        interval += hrs;
    }

    /* Minutes was specified */
    if(match.length(3) > 0) {
        auto minutes = std::chrono::minutes(std::stoi(match[3]));

        if(match.length(1) > 0 && minutes >= std::chrono::days(1)) {
            std::cerr << "Invalid interval specification: minutes exceeds " 
                      << std::chrono::duration_cast<std::chrono::minutes>(std::chrono::days(1)).count() - 1
                      << " when days also specified."
                      << std::endl;
            exit(EXIT_FAILURE);
        }

        if(match.length(2) > 0 && minutes >= std::chrono::hours(1)) {
            std::cerr << "Invalid interval specification: minutes exceeds " 
                      << std::chrono::duration_cast<std::chrono::minutes>(std::chrono::hours(1)).count() - 1
                      << " when hours also specified."
                      << std::endl;
            exit(EXIT_FAILURE);
        }

        interval += minutes;
    }

    /* Minutes was seconds */
    if(match.length(4) > 0) {

        auto seconds = std::chrono::seconds(std::stoi(match[4]));


        if(match.length(1) > 0 && seconds >= std::chrono::days(1)) {
            std::cerr << "Invalid interval specification: seconds exceeds " 
                      << std::chrono::duration_cast<std::chrono::seconds>(std::chrono::days(1)).count() - 1 
                      << " when days also specified."
                      << std::endl;
            exit(EXIT_FAILURE);
        }

        if(match.length(2) > 0 && seconds >= std::chrono::hours(1)) {
            std::cerr << "Invalid interval specification: seconds exceeds " 
                      << std::chrono::duration_cast<std::chrono::seconds>(std::chrono::hours(1)).count() - 1 
                      << " when hours also specified."
                      << std::endl;
            exit(EXIT_FAILURE);
        }

        if(match.length(3) > 0 && seconds >= std::chrono::minutes(1)) {
            std::cerr << "Invalid interval specification: seconds exceeds " 
                      << std::chrono::duration_cast<std::chrono::seconds>(std::chrono::minutes(1)).count() - 1 
                      << " when minutes also specified."
                      << std::endl;
            exit(EXIT_FAILURE);
        }


        interval += seconds;
    }

    /* Number of seconds in the interval */
    return interval;
}


/**
 * @brief Parse the provided config file and return a args struct with the
 *        provided values.
 * 
 * @param config_file 
 * @param agent_id 
 * @return struct args 
 */
static struct args parse_config_file(std::string_view config_file, 
                                     std::string_view agent_id) {

    using qs::common::parse_config; 
    using qs::common::ConfigType;

    try {

        /* Parse the yaml config file */
        auto config = parse_config<ConfigType::YAML>(config_file);

        
        /* Get the properties of the agent */
        auto properties = config.get_agent_properties_by_id(agent_id);

        /* Check that the id is a scan agent */
        if(properties.at("type") != "scan_agents") {   
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

        return {  .id            = std::move(properties.at("id")),
                  .directory     = std::move(properties.at("root_directory")),
                  .scan_interval = std::move(parse_interval(properties.at("interval"))), 
                  .nats_url      = std::move(nats_url),
                  .scan_stream   = std::move(queue_properties.at("stream_name")),
                  .scan_consumer = std::move(queue_properties.at("consumer_name")),
                  .scan_subject  = std::move(queue_properties.at("subject"))
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
 * @brief Parse the command line argumments and return a struct of the arguments.
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
        ("interval", po::value<std::string>(), "Scan interval of the form [#days][#hours][#minutes][#seconds], e.g. 1d2h3m4s, 2h4s, 4s")
        ("directory", po::value<std::string>(), "Top level directory to start scan");


        
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

    /* Verify that interval was specified either on the command line or in the
     * config */
    if(vm.count("interval") != 1 && vm.count("config") < 1) {
        std::cerr << "Error, must specify a scan interval" << std::endl;
        exit(EXIT_FAILURE);

    /* If it was specified on command line use that even if there was config file.*/
    } else if (vm.count("interval") == 1) {
        
        /* Parse the can interval */
        args.scan_interval = parse_interval(vm["interval"].as<std::string>());
    }

    /* Verify that a root directory was given either on command line or in a 
     * config file */
    if(vm.count("directory") != 1 && vm.count("config") < 1) {
        std::cerr << "Error, must specify a top level directory for scanning." << std::endl;
        exit(EXIT_FAILURE);

    /* If specified on the command line use that instead of the config file */
    } else if(vm.count("directory") == 1) {
        args.directory = vm["directory"].as<std::string>();
    }

    /* Verify that a stream was given either on command line or in a 
     * config file */
    if(vm.count("stream") != 1 && vm.count("config") < 1) {
        std::cerr << "Error, must specify a scan stream" << std::endl;
        exit(EXIT_FAILURE);

    /* If specified on the command line use that instead of the config file */
    } else if(vm.count("stream") == 1) {
        args.scan_stream = vm["stream"].as<std::string>();
    }

    /* Verify that a consumer was given either on command line or in a 
     * config file */
    if(vm.count("consumer")!= 1 && vm.count("config") < 1) {
        std::cerr << "Error, must specify a scan consumer" << std::endl;
        exit(EXIT_FAILURE);
    
    /* If specified on the command line use that instead of the config file */
    } else if(vm.count("consumer") == 1) {
        args.scan_consumer = vm["consumer"].as<std::string>();
    }

    /* Verify that a subject was given either on command line or in a 
     * config file */
    if(vm.count("subject") != 1 && vm.count("config") < 1) {
        std::cerr << "Error, must specify a scan subject" << std::endl;
        exit(EXIT_FAILURE);
    
    /* If specified on the command line use that instead of the config file */
    } else if(vm.count("subject") == 1) {
        args.scan_subject = vm["subject"].as<std::string>();
    }
       

    /* Return an arg struct of the arguments to the process */
    return std::move(args);
}

/**
 * @brief Start of the scan program.
 * 
 * @param argc 
 * @param argv 
 * @return int 
 */
int main(int argc, char *argv[]) {

    /* Process the command line */
    auto args = parse_commandline(argc, argv);

    /* Print the configuration */
    std::clog << "Scan agent id: " << args.id << std::endl;
    std::clog << "Nats server url: " << args.nats_url << std::endl;
    std::clog << "Scan stream: " << args.scan_stream << std::endl;
    std::clog << "Scan consumer: " << args.scan_consumer << std::endl;
    std::clog << "Scan subject: " << args.scan_subject << std::endl;
    std::clog << "Scan interval: " << args.scan_interval.count() << std::endl;
    std::clog << "Top level directory: " << args.directory << std::endl;
    
    std::clog << "Starting scan agent..." << std::endl;
    
    /* Create the messaging service */
    MsgService auto ms = create_messaging_service<messaging_services::JETSTREAM>(
                                    std::string_view(args.nats_url));

    /* Create the publisher for the scan queue*/
    MsgPublisher auto mq_publisher = 
                ms.create_queue_publisher(std::string_view(args.scan_stream), 
                                          std::string_view(args.scan_consumer), 
                                          std::string_view(args.scan_subject));

    /* Create the agent */
    scan_agent agent = create_scan_agent<scan_agents::LFS_FIND>(
                                    mq_publisher, 
                                    std::string_view(args.directory),
                                    args.scan_interval);

    /* Run the agent */
    agent.run();


    return EXIT_SUCCESS;
}
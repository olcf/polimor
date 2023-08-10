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
#include <cstdlib>
#include <vector>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <tuple>
#include <thread>
#include <mqueue.h>

#include "../scan_agents/scan_agent.h"
#include "../messaging/messaging.h"
#include "../messaging/details/posix_messaging_impl.h"

constexpr std::string_view scan_stream   = "scan";
constexpr std::string_view scan_consumer = "scan-files-consumer";
constexpr std::string_view scan_subject  = "scan.files.results";


static auto setup() {

   
    messaging_service ms = create_messaging_service<messaging_services::JETSTREAM>();

    message_queue_publisher mq_pub = ms.create_queue_publisher(scan_stream, scan_consumer, scan_subject);

    message_queue_subscriber mq_sub = ms.create_queue_subscriber(scan_stream, scan_consumer, scan_subject);

    auto tmpdir = std::filesystem::temp_directory_path() / "scan_agents_test";
    
    std::filesystem::create_directories(tmpdir);

    return std::tuple { std::move(ms), std::move(mq_pub), std::move(mq_sub), tmpdir };
}

static void teardown(std::filesystem::path tmpdir) {


    std::filesystem::remove_all(tmpdir);
}


static void simple_test() {

    /* Setup */
    auto [ms, mq_pub, mq_sub, tmpdir] = setup();
    

    std::vector<std::string> paths;

    for(auto &filename : {"file1", "file2", "file3"}) {
        
        auto entry = paths.emplace_back(tmpdir/filename);
        std::ofstream f(entry);
    }

    scan_agent agent = create_scan_agent<scan_agents::LFS_FIND>(mq_pub, tmpdir.c_str(), std::chrono::seconds(30));

    agent.run();

    
    /* Teardown */
    teardown(tmpdir);
}


/* Prototype for backdoor test function for creating lfs_find_scan_agent */
lfs_find_scan_agent_impl create_lfs_find_scan_agent(
                    const message_queue_publisher &mq_publisher, 
                    std::string_view path, 
                    std::chrono::seconds scan_interval,
                    std::string_view executable);

static void json_test() {

    /* Setup */
    auto [ ms, mq_pub, mq_sub, tmpdir] = setup();
    
    lfs_find_scan_agent_impl agent = create_lfs_find_scan_agent(mq_pub, 
                                             tmpdir.c_str(), 
                                             std::chrono::seconds(30),
                                             "./fake_lfs_find");

    //MsgSubscriber auto tmp_sub = mq_sub;

    std::thread t([&mq_sub = mq_sub](){ 

        while(true) {

            auto msg = mq_sub.receive<scan_message>();
        
            std::cout << "json_test: " 
                      << "  atime: " << std::chrono::system_clock::to_time_t(msg.atime)
                      << "  mtime: " << std::chrono::system_clock::to_time_t(msg.mtime)
                      << "  size: " << msg.size
                      << "  uid: " << msg.uid
                      << "  gid: " << msg.gid
                      << "  stripe_count: " << msg.stripe_count
                      << "  filesys: " << msg.filesys
                      << "  path: " << msg.path
                      << "  ost_pool: " << msg.ost_pool
                      << "  fid: " << msg.fid
                      << "  type: " << msg.type
                      << std::endl;
        }

    });

    agent.run();


    t.join();

    /* Teardown */
    teardown(tmpdir);
}


int main(int argc, const char* argv[]) {


    if(argc != 2) {
        std::cerr << "Usage: " << argv[0] << " test-number" << std::endl;
        return EXIT_SUCCESS;
    }

    int test_number = atoi(argv[1]);


    switch(test_number) {

        case 1:
            simple_test();
            break;

        case 2:
            json_test();
            break;

        default:
            std::cerr << "Invalid test number" << std::endl;
            return EXIT_FAILURE;
    }

    
    return EXIT_SUCCESS;
}
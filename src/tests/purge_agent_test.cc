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
#include <mqueue.h>
#include <iostream>
#include <fstream>
#include <string>
#include <string_view>
#include <thread>
#include <chrono>
#include <filesystem>
#include <vector>


#include "../purge_agents/purge_agents.h"

constexpr std::string_view removal_stream = "purge";
constexpr std::string_view removal_consumer = "purge-files-consumer";
constexpr std::string_view removal_subject = "purge.files.request";

int main(int argc, const char* argv[]) {

    std::vector<std::string> paths;

    auto tmpdir =  std::filesystem::temp_directory_path() / "purge_agents_test";
    std::filesystem::create_directories(tmpdir);

    for(auto &filename : {"file1", "file2", "file3"}) {
        
        auto entry = paths.emplace_back(tmpdir/filename);
        std::ofstream f(entry);
    }
    
    MsgService auto ms = create_messaging_service<messaging_services::JETSTREAM>();

    MsgPublisher auto mq_pub = ms.create_queue_publisher(removal_stream, removal_consumer, removal_subject);
    MsgSubscriber auto mq_sub = ms.create_queue_subscriber(removal_stream, removal_consumer, removal_subject);
    purge_agent agent = create_purge_agent(mq_sub);


    std::thread t([&mq_pub, &paths, &agent](){ 

        std::this_thread::sleep_for(std::chrono::seconds(10));

        while(true) {
            for(auto &path : paths) {
                mq_pub.send(purge_message(path));
            }


            std::this_thread::sleep_for(std::chrono::seconds(10));
        }
        
        agent.stop();
    });

    agent.run();



    std::filesystem::remove_all(tmpdir);


    return EXIT_SUCCESS;
}
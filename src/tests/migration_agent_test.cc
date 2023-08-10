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
#include <filesystem>
#include <iostream>
#include <fstream>
#include <string_view>
#include <thread>
#include <vector>

#include <mqueue.h>

#include "../messaging/messaging.h"
#include "../migration_agents/migration_agent.h"



constexpr std::string_view migration_queue_name = "migration.files.request";


int main(int argc, const char* argv[]) {

    std::vector<std::string> paths;

    std::string mq_name = std::string("/").append(migration_queue_name);

    mq_unlink(mq_name.c_str());


    auto tmpdir =  std::filesystem::temp_directory_path() / "purge_agents_test";
    std::filesystem::create_directories(tmpdir);

    for(auto &filename : {"file1", "file2", "file3"}) {
        
        auto entry = paths.emplace_back(tmpdir/filename);
        std::ofstream f(entry);
    }

    MsgService auto ms = create_messaging_service<messaging_services::POSIX>();

    MsgPublisher auto mq_pub = ms.create_queue_publisher(migration_queue_name);
    MsgSubscriber auto mq_sub = ms.create_queue_subscriber(migration_queue_name);

    migration_agent agent = create_migration_agent<migration_agents::LFS_MIGRATE>(mq_sub);


    std::thread t([&mq_pub, &paths](){ 

        std::this_thread::sleep_for(std::chrono::seconds(10));

        for(auto &path : paths) {
            migration_message msg(path);

            mq_pub.send(msg);
        }
    });

    agent.run();

    std::filesystem::remove_all(tmpdir);
    
    mq_unlink(mq_name.c_str());

    return EXIT_SUCCESS;
}
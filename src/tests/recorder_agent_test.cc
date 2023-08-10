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

#include "../messaging/messaging.h"
#include "../recording_agents/recording_agents.h"

constexpr std::string_view recorder_queue_name = "recorder";
constexpr std::string_view recorder_consumer = "recorder-files-consumer";
constexpr std::string_view recorder_subject = "recorder.files";


int main(int argc, const char* argv[]) {

    
    std::vector<std::string> messages = {
       "{ \"type\": \"d\",\"path\": \"/lustre/ldev/rmohr\",\"atime\": 1642662012,\"mtime\": 1642661471,\"size\": 4096,\"uid\": 6598,\"gid\": 9294,\"format\": {\"filesys\": \"lustre\",\"ost_pool\": \"\",\"stripe_count\": 0,\"fid\": \"0x200000403:0x295:0x0\" } }",
       "{ \"type\": \"d\",\"path\": \"/lustre/ldev/rmohr/dir1\",\"atime\": 1642661534,\"mtime\": 1642661594,\"size\": 4096,\"uid\": 6598,\"gid\": 9294,\"format\": {\"filesys\": \"lustre\",\"ost_pool\": \"\",\"stripe_count\": 0,\"fid\": \"0x240000403:0xa:0x0\" } }",
       "{ \"type\": \"d\",\"path\": \"/lustre/ldev/rmohr/dir1/subdir1\",\"atime\": 1642662138,\"mtime\": 1642661593,\"size\": 4096,\"uid\": 6598,\"gid\": 9294,\"format\": {\"filesys\": \"lustre\",\"ost_pool\": \"\",\"stripe_count\": 0,\"fid\": \"0x280000403:0x21:0x0\" } }",
       "{ \"type\": \"d\",\"path\": \"/lustre/ldev/rmohr/dir1/subdir2\",\"atime\": 1642661656,\"mtime\": 1642661652,\"size\": 4096,\"uid\": 6598,\"gid\": 9294,\"format\": {\"filesys\": \"lustre\",\"ost_pool\": \"\",\"stripe_count\": 0,\"fid\": \"0x200000403:0x296:0x0\" } }",
       "{ \"type\": \"f\",\"path\": \"/lustre/ldev/rmohr/dir1/subdir2/checkpoint.1\",\"atime\": 1609553580,\"mtime\": 1609553580,\"size\": 10485760,\"uid\": 6598,\"gid\": 9294,\"format\": {\"filesys\": \"lustre\",\"ost_pool\": \"\",\"stripe_count\": 3,\"fid\": \"0x200000403:0x297:0x0\" } }",
       "{ \"type\": \"f\",\"path\": \"/lustre/ldev/rmohr/dir1/subdir2/checkpoint.2\",\"atime\": 1642661652,\"mtime\": 1642661687,\"size\": 3145728,\"uid\": 6598,\"gid\": 9294,\"format\": {\"filesys\": \"lustre\",\"ost_pool\": \"\",\"stripe_count\": 2,\"fid\": \"0x200000403:0x298:0x0\" } }",
       "{ \"type\": \"f\",\"path\": \"/lustre/ldev/rmohr/dir1/output.txt\",\"atime\": 1633093200,\"mtime\": 1633093200,\"size\": 54272,\"uid\": 6598,\"gid\": 9294,\"format\": {\"filesys\": \"lustre\",\"ost_pool\": \"\",\"stripe_count\": 3,\"fid\": \"0x240000403:0xc:0x0\" } }",
       "{ \"type\": \"f\",\"path\": \"/lustre/ldev/rmohr/dir1/input-data\",\"atime\": 1642661510,\"mtime\": 1592936581,\"size\": 10485760,\"uid\": 6598,\"gid\": 9294,\"format\": {\"filesys\": \"lustre\",\"ost_pool\": \"\",\"stripe_count\": 1,\"fid\": \"0x240000403:0xb:0x0\" } }",
   };

    std::string mq_name = std::string("/").append(queue_name);

    mq_unlink(mq_name.c_str());
    
    
    MsgService auto ms = create_messaging_service<messaging_services::POSIX>();

    MsgPublisher auto  mq_pub = ms.create_queue_publisher(queue_name);
    MsgSubscriber auto mq_sub = ms.create_queue_subscriber(queue_name);

    recording_agent *agent = create_recording_agent<RECORDING_AGENTS::SQLITE>(
        ms, mq_sub);

    std::thread t([&mq_pub, &messages](){ 

        json_deserializer_impl<recorder_message> deserializer;

        std::this_thread::sleep_for(std::chrono::seconds(10));

        for(auto &message : messages) {
            mq_pub.send<recorder_message>(
                deserializer(message));
        }
    });

    agent->run();

    // std::filesystem::remove_all(tmpdir);

    mq_unlink(mq_name.c_str());

    return EXIT_SUCCESS;
}
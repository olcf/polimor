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
#include <vector>
#include <thread>
#include <chrono>

#include <mqueue.h>

#include "../messaging/messaging.h"
#include "../policy_engine/policy_engine.h"
#include "../messaging/details/message_json_deserializer_boost_impl.h"

constexpr std::string_view scan_queue_name  = "scan.files.results";
constexpr std::string_view removal_queue_name = "purge.files.request";
constexpr std::string_view migration_queue_name = "migration.files.request";
constexpr std::string_view recorder_queue_name = "recorder.files";

int main(int argc, const char* argv[]) {


    std::vector<std::string> messages = {
        "{ \"type:\": \"d\", \"path\": \"/lustre/ldev/rmohr\", \"atime\": 1642662012, \"mtime\": 1642661471, \"size\": 4096, \"uid\": 6598, \"gid\": 9294, \"format\": { \"filesys\": \"lustre\", \"ost_pool\": \"\", \"stripe_count\": 0, \"fid\": \"0x200000403:0x295:0x0\" } }",
        "{ \"type\": \"d\", \"path\": \"/lustre/ldev/rmohr/dir1\", \"atime\": 1642661534, \"mtime\": 1642661594, \"size\": 4096, \"uid\": 6598, \"gid\": 9294, \"format\": { \"filesys\": \"lustre\", \"ost_pool\": \"\", \"stripe_count\": 0, \"fid\": \"0x240000403:0xa:0x0\" } }",
        "{ \"type\": \"d\", \"path\": \"/lustre/ldev/rmohr/dir1/subdir1\", \"atime\": 1642662138, \"mtime\": 1642661593, \"size\": 4096, \"uid\": 6598, \"gid\": 9294, \"format\": { \"filesys\": \"lustre\", \"ost_pool\": \"\", \"stripe_count\": 0, \"fid\": \"0x280000403:0x21:0x0\" } }",
        "{ \"type\": \"d\", \"path\": \"/lustre/ldev/rmohr/dir1/subdir2\", \"atime\": 1642661656, \"mtime\": 1642661652, \"size\": 4096, \"uid\": 6598, \"gid\": 9294, \"format\": { \"filesys\": \"lustre\", \"ost_pool\": \"\", \"stripe_count\": 0, \"fid\": \"0x200000403:0x296:0x0\" } }",
        "{ \"type\": \"f\", \"path\": \"/lustre/ldev/rmohr/dir1/subdir2/checkpoint.1\", \"atime\": 1609553580, \"mtime\": 1609553580, \"size\": 10485760, \"uid\": 6598, \"gid\": 9294, \"format\": { \"filesys\": \"lustre\", \"ost_pool\": \"\", \"stripe_count\": 3, \"fid\": \"0x200000403:0x297:0x0\" } }",
        "{ \"type\": \"f\", \"path\": \"/lustre/ldev/rmohr/dir1/subdir2/checkpoint.2\", \"atime\": 1642661652, \"mtime\": 1642661687, \"size\": 3145728, \"uid\": 6598, \"gid\": 9294, \"format\": { \"filesys\": \"lustre\", \"ost_pool\": \"\", \"stripe_count\": 2, \"fid\": \"0x200000403:0x298:0x0\" } }",
        "{ \"type\": \"f\", \"path\": \"/lustre/ldev/rmohr/dir1/output.txt\", \"atime\": 1633093200, \"mtime\": 1633093200, \"size\": 54272, \"uid\": 6598, \"gid\": 9294, \"format\": { \"filesys\": \"lustre\", \"ost_pool\": \"\", \"stripe_count\": 3, \"fid\": \"0x240000403:0xc:0x0\" } }",
        "{ \"type\": \"f\", \"path\": \"/lustre/ldev/rmohr/dir1/input-data\", \"atime\": 1642661510, \"mtime\": 1592936581, \"size\": 10485760, \"uid\": 6598, \"gid\": 9294, \"format\": { \"filesys\": \"lustre\", \"ost_pool\": \"\", \"stripe_count\": 1, \"fid\": \"0x240000403:0xb:0x0\" } }",
   };

   std::string mq_scan_path = std::string("/").append(scan_queue_name);
   std::string mq_removal_path = std::string("/").append(removal_queue_name); 
   std::string mq_migration_path = std::string("/").append(migration_queue_name);
   std::string mq_recorder_path = std::string("/").append(recorder_queue_name);

    mq_unlink(mq_scan_path.c_str());
    mq_unlink(mq_removal_path.c_str());
    mq_unlink(mq_migration_path.c_str());
    mq_unlink(mq_recorder_path.c_str());

    try {

        MsgService auto ms = create_messaging_service<messaging_services::POSIX>();

        MsgSubscriber auto scan_mq_sub = ms.create_queue_subscriber(scan_queue_name);
        MsgSubscriber auto removal_mq_sub = ms.create_queue_subscriber(removal_queue_name);


        MsgPublisher auto scan_mq_pub = ms.create_queue_publisher(scan_queue_name);
        MsgPublisher auto removal_mq_pub = ms.create_queue_publisher(removal_queue_name);
        MsgPublisher auto migration_mq_pub = ms.create_queue_publisher(migration_queue_name);
        MsgPublisher auto recorder_mq_pub = ms.create_queue_publisher(recorder_queue_name);



        policy_engine *policy_engine = create_policy_engine(scan_mq_sub, 
                                                            removal_mq_pub,
                                                            migration_mq_pub,
                                                            recorder_mq_pub);


        std::thread scan_thread([&scan_mq_pub, &messages](){ 

            json_deserializer_impl<scan_message> deserializer;

            std::this_thread::sleep_for(std::chrono::seconds(10));

            for(auto &message : messages) {
                scan_mq_pub.send(deserializer(message));
            }
        });

        std::thread removal_thread([&removal_mq_sub, &messages](){ 

            while(true) {
                auto msg = removal_mq_sub.receive<purge_message>();

                std::clog << "Deletion message: " << msg.path << std::endl;
            }
        });

        policy_engine->run();

        
    }catch(const std::exception &e) {
        std::cerr << "Error :" << e.what() << std::endl;
    }

    mq_unlink(mq_scan_path.c_str());
    mq_unlink(mq_removal_path.c_str());
    mq_unlink(mq_migration_path.c_str());
    mq_unlink(mq_recorder_path.c_str());



    return EXIT_SUCCESS;

}
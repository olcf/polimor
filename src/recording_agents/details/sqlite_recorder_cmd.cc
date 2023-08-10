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
#include <string>
#include <string_view>

#include <boost/program_options.hpp>
#include <sqlite3.h>

#include "../../messaging/messaging.h"
#include "../../messaging/details/jetstream_messaging_impl.h"


std::string_view create_table_sql = 
    "CREATE TABLE Records(\
        path TEXT PRIMARY KEY, type TEXT, atime INTEGER, mtime INTEGER, \
        size INTEGER, uid INTEGER, gid INTEGER, filesys TEXT, \
        ost_pool TEXT, stripe_count INTEGER, fid TEXT, \
        timestamp INTEGER)";

std::string_view insert_into_sql = 
    "INSERT OR REPLACE INTO Records( \
        path, type, atime, mtime, size, uid, gid, filesys, ost_pool, stripe_count, fid, timestamp) \
    VALUES(?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9, ?10, ?11, CURRENT_TIMESTAMP)";



struct args {
    std::string nats_url;
    std::string db_name;
    std::string mq_name;
};

/**
 * @brief Parse the command line and return the processed arguments.
 * 
 * @param argc 
 * @param argv 
 * @return struct args 
 */
static struct args parse_commandline(int argc, char *argv[]) {

    namespace po = boost::program_options;

    std::string nats_url;

    /* Build the description of the command line */
    po::options_description desc("Options");
    desc.add_options()
        ("help,h", "This help message")
        ("nats_server,n", po::value<std::vector<std::string>>(), "URL to the NATS server (may be repeated), e.g. rage1.ccs.ornl.gov:4222")
        ("db_name,d", po::value<std::string>(), "Path to the database file to use")
        ("mq_name,m", po::value<std::string>(), "Name of the message queue to use");

    /* Progress the command line */
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm); 

    if (vm.count("help")) {
        std::cerr << desc << std::endl;
        exit(EXIT_SUCCESS);
    }

    if(vm.count("nats_server") < 1) {
        std::cerr << "Error, must specify at least one NATS server." << std::endl;
        exit(EXIT_FAILURE);
    }

    /* Join the nats servers into one NATS url string */
    for(auto &nats_server : vm["nats_server"].as<std::vector<std::string>>()) {
        nats_url += (nats_url.empty()) ? std::string_view("") : std::string_view(",");
        nats_url += "nats://" + nats_server;
    }

    if(vm.count("db_name") < 1) {
        std::cerr << "Error, must specify a database file" << std::endl;
        exit(EXIT_FAILURE);
    }


    if(vm.count("mq_name") < 1) {
        std::cerr << "Error, must specify a message queue" << std::endl;
        exit(EXIT_FAILURE);
    }


    /* Return an arg struct of the arguments to the process */
    return { std::move(nats_url), 
             std::move(vm["db_name"].as<std::string>()), 
             std::move(vm["mq_name"].as<std::string>()) };
}


int main(int argc, char *argv[]) {


    auto args = parse_commandline(argc, argv);

    MsgService auto ms = create_messaging_service<messaging_services::JETSTREAM>(std::string_view(args.nats_url));

    MsgSubscriber auto mq_sub = ms.create_queue_subscriber(args.mq_name);

    sqlite3 *db = nullptr;
    sqlite3_stmt *ppStmt = nullptr;

    int rc = sqlite3_open(args.db_name.c_str(), &db);
    
    if (rc != SQLITE_OK)  {
        std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
    }

    char *err_msg = nullptr;

    rc = sqlite3_exec(db, create_table_sql.data(), nullptr, nullptr, &err_msg);

    if (rc != SQLITE_OK) {
        std::cerr << "Can not create table: " << err_msg << std::endl;
        sqlite3_free(err_msg); 
    }


    rc = sqlite3_prepare_v3(db, insert_into_sql.data(), -1, SQLITE_PREPARE_PERSISTENT, &ppStmt, nullptr);
  
    if (rc != SQLITE_OK) {
        std::cerr << "Error preparing sqlite statement: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
    }

    while(true) { 

        try {
            auto msg = mq_sub.receive<recorder_message>();

            sqlite3_bind_text(ppStmt, 1, msg.path.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(ppStmt, 2, &msg.type, -1, SQLITE_TRANSIENT);
            sqlite3_bind_int64(ppStmt, 3, std::chrono::system_clock::to_time_t(msg.atime));
            sqlite3_bind_int64(ppStmt, 4, std::chrono::system_clock::to_time_t(msg.mtime));
            sqlite3_bind_int64(ppStmt, 5, msg.size);
            sqlite3_bind_int64(ppStmt, 6, msg.uid);
            sqlite3_bind_int64(ppStmt, 7, msg.gid);
            sqlite3_bind_text(ppStmt, 8, msg.filesys.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(ppStmt, 9, msg.ost_pool.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int64(ppStmt, 10, msg.stripe_count);
            sqlite3_bind_text(ppStmt, 11, msg.fid.c_str(), -1, SQLITE_TRANSIENT);
        
            rc = sqlite3_step(ppStmt);

            if(rc != SQLITE_DONE) {
                std::cerr << "Error inserting record: " << sqlite3_errmsg(db) << std::endl;
                sqlite3_close(db);
            }

        } catch(const std::exception &e) {
            std::cerr << "Error receiving message: " << e.what();
        }
        
        sqlite3_reset(ppStmt);
    }

    sqlite3_finalize(ppStmt);
    sqlite3_close(db);

    return EXIT_SUCCESS;
}
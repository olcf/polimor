/****************************************************************************
 * Copyright 2023 UT Battelle, LLC
 *
 * This work was supported by the Oak Ridge Leadership Computing Facility at
 * the Oak Ridge National Laboratory, which is managed by UT Battelle, LLC for
 * the U.S. DOE (under the contract No. DE-AC05-00OR22725).
 *
 * This file is part of the PoliMOR project.
 ****************************************************************************/


#include <functional>
#include <iostream>
#include <string>
#include <string_view>
#include <thread>

//#include "../migration_agent.h"
//#include "../../messaging/messaging.h"
#include "./lfs_migration_migration_agent_impl.h"
#include "../../common/process_control.h"

void lfs_migrate_migration_agent_impl::run() {

    std::clog << "Migration agent(" << std::this_thread::get_id() <<"): "
                << "Running..." << std::endl;


    std::vector<std::string> process_args = (_dry_run) ? 
        std::move(std::vector<std::string> { "/bin/echo" }) :
        std::move(std::vector<std::string> { this->_executable, "migrate", "-p", "capacity" });
        
    while(this->_stop == false) {

        try {
            auto msg = _mq_sub.receive<migration_message>();

            std::clog << "Migration agent(" << std::this_thread::get_id() <<"): " 
                    << "Received message" << std::endl;


            /* TODO need to have a way specifing the destination pool,
            * right now it is just "capacity" */

            /* Build a reference array for passing the temp args */
            /* TODO look at boost::static_vector for stack storage array */
            std::vector<std::string> args { process_args.begin(), process_args.end() };
            args.emplace_back(msg.path);

            process migration_process(args);
            
            std::basic_filebuf<char> filebuf = migration_process.launch();
        
            /* Read line by line until there is no more data */
            for(std::string buffer; std::getline(std::istream(&filebuf), buffer);) {
                std::clog << "output: " << buffer << std::endl;
            }

            migration_process.wait(); 

        } catch(const std::exception &e) {
            std::clog << "Error handling message: " << e.what() << std::endl;
        }
    }
}

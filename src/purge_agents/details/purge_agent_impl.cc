/****************************************************************************
 * Copyright 2023 UT Battelle, LLC
 *
 * This work was supported by the Oak Ridge Leadership Computing Facility at
 * the Oak Ridge National Laboratory, which is managed by UT Battelle, LLC for
 * the U.S. DOE (under the contract No. DE-AC05-00OR22725).
 *
 * This file is part of the PoliMOR project.
 ****************************************************************************/


#include <string_view>
#include <thread>

#include "./purge_agent_impl.h"
#include "../../common/process_control.h"
#include "../../messaging/messaging.h"




void purge_agent_impl::run() {

    std::clog << "Purge agent(" << std::this_thread::get_id() <<"): "
                << "Running..." << std::endl;

    /* Initial the process args to have the command for echo when doing a dry run
     * or rm for regular runs. */
    std::vector<std::string> process_args = (_dry_run) ? 
        std::move(std::vector<std::string>{ "/bin/echo" }) : 
        std::move(std::vector<std::string>{ "/bin/rm", "-f" });

    while(_stop == false) {

        try {
            auto msg = _mq_sub.receive<purge_message>();

        
            std::clog << "Purge agent(" << std::this_thread::get_id() <<"): " 
                        << "Received message" << std::endl;
        
            std::clog << "Purge agent(" << std::this_thread::get_id() <<"): "
                        << "Asked to remove " << msg.path << std::endl;

            
            /* Build a reference array for passing temp args */
            /* TODO look at boost::static_vector for stack storage array */
            std::vector<std::reference_wrapper<const std::string>> args{process_args.begin(), process_args.end()}; 
            args.emplace_back(msg.path);
                
            process removal_process(args);
            
            
            std::basic_filebuf<char> filebuf = removal_process.launch();
        

            /* Read line by line until there is no more data */
            for(std::string buffer; std::getline(std::istream(&filebuf), buffer);) {
                std::clog << "output: " << buffer << std::endl;
            }

            removal_process.wait();

        /* TODO need a special deserialization message and to handle
            * other error exceptions */
        } catch (const std::exception &e) {
            std::clog << "Purge agent(" << std::this_thread::get_id() << "): "
                        << "Error receiving msg: " << e.what();
        }                
    }
}


    

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
#include <fstream>

#include <string_view>
#include <system_error>
#include <thread>
#include <chrono>

#include <unistd.h>
#include <fcntl.h>
#include <spawn.h>
#include <signal.h>
#include <sys/wait.h>
#include <ext/stdio_filebuf.h>



#include "../../common/process_control.h"
#include "./lfs_find_scan_agent.h"

    

void lfs_find_scan_agent_impl::run() {

    json_deserializer_impl<scan_message> deserializer;

    std::clog << "Scan agent(" << std::this_thread::get_id() <<"): "
                << "Running..." << std::endl;

    while(this->_stop == false) {
        //_launch_executable();

        process scan_process(_executable, 
            "find", _path,  "--printf",  
            "{ \"type\": \"%y\", \"path\": \"%p\", \"atime\": %A@, \"mtime\": %T@, \"size\": %s, \"uid\": %U, \"gid\": %G, \"format\": { \"filesys\": \"lustre\", \"ost_pool\": \"%Lp\", \"stripe_count\": %Lc, \"fid\": \"%LF\" } }\n");

        // __gnu_cxx::stdio_filebuf<char> filebuf(1, std::ios::in);

        // std::istream input(&filebuf);

        std::basic_filebuf<char> filebuf = scan_process.launch();

        /* Read line by line until there is no more data */
        for(std::string buffer; std::getline(std::istream(&filebuf), buffer);) {

            std::clog << buffer << std::endl;

            /* TODO verify format of message and look for error messages from lfs_find */
            try {
                
                auto msg = deserializer(buffer);

                /* send data to message queue */
                _mq_pub.send(msg);

            /* TODO need to make a deserialize exception */
            } catch (const std::exception& e) {
                std::clog << "Error deserializing and sending message: " << buffer << ": "
                            << e.what() << std::endl;
            }
        }

        std::this_thread::sleep_for(_scan_interval);
    }
}




/* Backdoor creation function for testing */
lfs_find_scan_agent_impl create_lfs_find_scan_agent(
                    const message_queue_publisher &mq_publisher, 
                    std::string_view path, 
                    std::chrono::seconds scan_interval,
                    std::string_view executable) {

    return lfs_find_scan_agent_impl(mq_publisher, path, scan_interval, executable);
}
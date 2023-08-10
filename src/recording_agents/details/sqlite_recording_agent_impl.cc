/****************************************************************************
 * Copyright 2023 UT Battelle, LLC
 *
 * This work was supported by the Oak Ridge Leadership Computing Facility at
 * the Oak Ridge National Laboratory, which is managed by UT Battelle, LLC for
 * the U.S. DOE (under the contract No. DE-AC05-00OR22725).
 *
 * This file is part of the PoliMOR project.
 ****************************************************************************/

#include <array>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <regex>
#include <string>
#include <string_view>
#include <thread>

#include <openssl/sha.h>
#include <vector>

#include "../../messaging/messaging.h"
#include "../recording_agents.h"


class sqlite_recording_agent_impl: public recording_agent {

    private:
        messaging_service _ms;
        message_queue_subscriber _mq_sub;
        bool _stop;

        const static std::regex _message_regex;

    public:
        sqlite_recording_agent_impl(const messaging_service &ms, 
                                    const message_queue_subscriber &mq_sub) : 
            _ms(ms), _mq_sub(mq_sub), _stop(false) { }
        sqlite_recording_agent_impl(const sqlite_recording_agent_impl &) = delete;
        sqlite_recording_agent_impl(sqlite_recording_agent_impl &&) = delete;
        sqlite_recording_agent_impl &operator=(const sqlite_recording_agent_impl &) = delete;
        sqlite_recording_agent_impl &operator=(sqlite_recording_agent_impl &&) = delete;
        ~sqlite_recording_agent_impl() = default;


        void run();
        void stop();
};




void sqlite_recording_agent_impl::run()
{
    std::clog << "Sqlite recording agent(" << std::this_thread::get_id() <<"): "
                      << "Running..." << std::endl;


    /* Creates queues */
    std::array db_queues = {
         _ms.create_queue_publisher(std::string_view("/db"+std::to_string(0))), 
         _ms.create_queue_publisher(std::string_view("/db"+std::to_string(1))), 
         _ms.create_queue_publisher(std::string_view("/db"+std::to_string(2))), 
    };


    while(_stop == false) {

        try {
            auto msg= _mq_sub.receive<recorder_message>();


            std::clog << "Sqlite recording agent(" << std::this_thread::get_id() <<"): " 
                         " Received message: " << std::endl;


            unsigned char md[SHA_DIGEST_LENGTH];
        

            /* Compute SHA1 sum of the path */
            SHA1(reinterpret_cast<const unsigned char *>(msg.path.c_str()), 
                  msg.path.length(), md);
               
        
            uint16_t result = 0; // Just in case, to prevent overflow
            uint8_t num_nodes = 3;

            /* Compute the the modulo of the SHA1 sum */
            std::clog << "SHA1: ";
            for(unsigned char c : md) {
                std::clog << std::hex << (int)c << " ";

                /* TODO this method of modulo arthimetic needs to be 
                 * verfied */
                // a % b = ((a // 256) % b) * (256 % b) + (a % 256) % b
                result *= (256 % num_nodes);
                result %= num_nodes;
                result += (c % num_nodes);
                result %= num_nodes;
            }

            std::clog << std::endl;
            std::clog << "Modulo = " << result << std::endl;


             /* Send message to a database queue indexed at the modulo 
               of the sha1 sum */
            db_queues[result].send(msg);

        } catch (const std::exception &e) {
            std::clog << "Sqlite recording agent(" << std::this_thread::get_id() <<"): "
                      << "Caught exception on msg receive: " << e.what();
        }
    }
}

void sqlite_recording_agent_impl::stop(){
    this->_stop = true;
}


template<>
recording_agent *create_recording_agent<RECORDING_AGENTS::SQLITE>(
    messaging_service &ms, message_queue_subscriber &mq_sub) {

    return new sqlite_recording_agent_impl(ms, mq_sub);
}
 
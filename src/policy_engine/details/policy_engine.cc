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
#include <initializer_list>
#include <string>
#include <iostream>
#include <algorithm>
#include <string_view>
#include <array>
#include <thread>
#include <chrono>
#include <ranges>

#include <unistd.h>

#include "../../messaging/messaging.h"
#include "../policy_engine.h"


class policy_engine_impl : public policy_engine {

    public:
        policy_engine_impl() = delete;
        policy_engine_impl(const MsgSubscriber auto &scan_mq_sub, 
                           const MsgPublisher auto &removal_mq_pub, 
                           const MsgPublisher auto &migration_mq_pub, 
                           const MsgPublisher auto &recorder_mq_pub) noexcept: 
            _scan_mq_sub(scan_mq_sub), _removal_mq_pub(removal_mq_pub), 
            _migration_mq_pub(migration_mq_pub), _recorder_mq_pub(recorder_mq_pub) {};

        policy_engine_impl(policy_engine_impl &&) = delete;
        policy_engine_impl(const policy_engine_impl &) = delete;
        
        policy_engine_impl &operator=(policy_engine_impl &&) = delete;
        policy_engine_impl &operator=(const policy_engine_impl &) = delete;
        
        ~policy_engine_impl();

        void run();
        void stop();

    private:

        message_queue_subscriber _scan_mq_sub;
        message_queue_publisher _removal_mq_pub;
        message_queue_publisher _migration_mq_pub;
        message_queue_publisher _recorder_mq_pub;   
};




policy_engine_impl::~policy_engine_impl() {

}




void policy_engine_impl::run() {
    
    std::clog << "Policy engine(" << std::this_thread::get_id() <<"): "
              << "Running..." << std::endl;

    
    while(true) {

        std::vector<scan_message> messages;

        /* Get the messages and add them to the list of messages */
        try {
            
            auto message = _scan_mq_sub.receive<scan_message>();

             //this->_recorder_mq.send(message);

            messages.emplace_back(std::move(message));

        } catch (const std::exception &e) {
            std::clog << "Policy engine(" << std::this_thread::get_id() <<"): " 
                      << "Error receiving message: " << e.what() << std::endl;
        }

        /* Create the time mark for the filter to judge what to purge */
        std::chrono::time_point migration_timepoint = 
            std::chrono::system_clock::now() - std::chrono::days(2);

        std::chrono::time_point removal_timepoint = 
            std::chrono::system_clock::now() - std::chrono::days(30);


        for(auto const &msg : messages) {
            
            /* Deletion filter */
            if(msg.type == 'f' && msg.atime < removal_timepoint) {

                std::clog << "Policy engine(" << std::this_thread::get_id() <<"): " 
                          << "Has decided that " << msg.path << " needs deletion" << std::endl;
                
                this->_removal_mq_pub.send(purge_message(msg.path));

            /* Migration filter */
            }else if(msg.type == 'f' && msg.atime < migration_timepoint &&
                     msg.ost_pool.compare("performance") == 0) {

                std::clog << "Policy engine(" << std::this_thread::get_id() <<"): " 
                          << "Has decided that " << msg.path << " needs migration" << std::endl;
                
                this->_migration_mq_pub.send(migration_message(msg.path));
            } 
        }
    }
}

void policy_engine_impl::stop() {
    
}



policy_engine *create_policy_engine(const message_queue_subscriber &scan_mq_sub, 
                                    const message_queue_publisher &removal_mq_pub,
                                    const message_queue_publisher &migration_mq_pub,
                                    const message_queue_publisher &recorder_mq_pub) {

    return new policy_engine_impl(scan_mq_sub, removal_mq_pub, migration_mq_pub, 
                                  recorder_mq_pub);
}
/****************************************************************************
 * Copyright 2023 UT Battelle, LLC
 *
 * This work was supported by the Oak Ridge Leadership Computing Facility at
 * the Oak Ridge National Laboratory, which is managed by UT Battelle, LLC for
 * the U.S. DOE (under the contract No. DE-AC05-00OR22725).
 *
 * This file is part of the PoliMOR project.
 ****************************************************************************/

#ifndef __SCAN_AGENT_H__
#define __SCAN_AGENT_H__

#include <string>
#include <variant>

#include "./details/lfs_find_scan_agent.h"


enum class scan_agents {
    LFS_FIND,
};

class scan_agent : std::variant<lfs_find_scan_agent_impl> {

    public:
        /* No default constructor */
        scan_agent() = delete;

        /* Use the variant constructors */
        using variant::variant;
    
        virtual ~scan_agent() = default;

        void run() {
            std::visit([](auto &&impl) { 
                    impl.run(); 
                }, *static_cast<variant *>(this));
        }

        void stop() {
            std::visit([](auto &&impl) { 
                    impl.stop(); 
                }, *static_cast<variant *>(this));
        }
};


/* Template function for creating scanning agents. Template value needs to be 
   one of scan agents in the enum scan agents */
template<scan_agents> 
scan_agent create_scan_agent(const message_queue_publisher &mq_publisher, 
                             std::string_view path,
                             std::chrono::seconds scan_interval);






#endif // __SCAN_AGENT_H__
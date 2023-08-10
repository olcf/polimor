/****************************************************************************
 * Copyright 2023 UT Battelle, LLC
 *
 * This work was supported by the Oak Ridge Leadership Computing Facility at
 * the Oak Ridge National Laboratory, which is managed by UT Battelle, LLC for
 * the U.S. DOE (under the contract No. DE-AC05-00OR22725).
 *
 * This file is part of the PoliMOR project.
 ****************************************************************************/

#ifndef __MIGRATION_AGENT_H__
#define __MIGRATION_AGENT_H__

#include "../agents/agents.h"
#include "../messaging/messaging.h"
#include "./details/lfs_migration_migration_agent_impl.h"

enum class migration_agents {
    LFS_MIGRATE,
};


class migration_agent : std::variant<lfs_migrate_migration_agent_impl>, public agent {

    public:

        /* No default constructor */
        migration_agent() = delete;

        /* Use the variant constructors */
        using variant::variant;

        
        virtual ~migration_agent() = default;    

        void run() override {
            std::visit([](auto &&impl) { impl.run(); }, 
                        *static_cast<variant *>(this));
        }

        void stop() override {
            std::visit([](auto &&impl) { impl.stop(); }, 
                        *static_cast<variant *>(this));
        }
};


template<migration_agents>
migration_agent create_migration_agent(message_queue_subscriber &mq_sub, 
                                       bool dry_run=false);




#endif // __MIGRATION_AGENT_H__
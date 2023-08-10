/****************************************************************************
 * Copyright 2023 UT Battelle, LLC
 *
 * This work was supported by the Oak Ridge Leadership Computing Facility at
 * the Oak Ridge National Laboratory, which is managed by UT Battelle, LLC for
 * the U.S. DOE (under the contract No. DE-AC05-00OR22725).
 *
 * This file is part of the PoliMOR project.
 ****************************************************************************/

#pragma once


#include "../../messaging/messaging.h"

class lfs_migrate_migration_agent_impl {

    private:
        message_queue_subscriber _mq_sub;
        std::string _executable;
        bool _dry_run;
        bool _stop = false;

    public:
        lfs_migrate_migration_agent_impl(
                const message_queue_subscriber &mq_sub, 
                bool dry_run=false, 
                std::string_view executable="/usr/bin/lfs") :
            _mq_sub(mq_sub), _dry_run(dry_run), _executable(executable) {}

        lfs_migrate_migration_agent_impl(const lfs_migrate_migration_agent_impl &) = delete;
        lfs_migrate_migration_agent_impl &operator=(const lfs_migrate_migration_agent_impl &) = delete;
        
        lfs_migrate_migration_agent_impl(lfs_migrate_migration_agent_impl &&) = default;
        lfs_migrate_migration_agent_impl &operator=(lfs_migrate_migration_agent_impl &&) = default;
        
        virtual ~lfs_migrate_migration_agent_impl() = default;


        void run();

        void stop() {
            this->_stop = true;
        }
};
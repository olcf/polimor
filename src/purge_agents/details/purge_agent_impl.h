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

class purge_agent_impl {

    private:
        message_queue_subscriber _mq_sub;
        bool _dry_run;
        bool _stop;

    public:
        purge_agent_impl() = delete;

        purge_agent_impl(const message_queue_subscriber &mq_sub, 
                         bool dry_run=false) : 
            _mq_sub(mq_sub), _stop(false), _dry_run(dry_run) { }

        purge_agent_impl(const purge_agent_impl &other) = delete;
        purge_agent_impl &operator=(const purge_agent_impl &other) = delete;

        purge_agent_impl(purge_agent_impl &&other) = default;
        purge_agent_impl &operator=(purge_agent_impl &&other) = default;


        ~purge_agent_impl() = default;

        void run();

        void stop() {
            this->_stop = true;
        }
};
/****************************************************************************
 * Copyright 2023 UT Battelle, LLC
 *
 * This work was supported by the Oak Ridge Leadership Computing Facility at
 * the Oak Ridge National Laboratory, which is managed by UT Battelle, LLC for
 * the U.S. DOE (under the contract No. DE-AC05-00OR22725).
 *
 * This file is part of the PoliMOR project.
 ****************************************************************************/

#ifndef __PURGE_AGENTS_H__
#define __PURGE_AGENTS_H__

#include <boost/mpl/bool_fwd.hpp>
#include <variant>

#include "../agents/agents.h"
#include "../messaging/messaging.h"
#include "./details/purge_agent_impl.h"



class purge_agent : std::variant<purge_agent_impl>, public agent {

    public:

        /* No default constructor */
        purge_agent() = delete;

        /* Use the variant constructors */
        using variant::variant;

        virtual ~purge_agent() = default;

        void run() override {
            std::visit([](auto &&impl) {impl.run();}, 
                      *static_cast<variant *>(this));
        }

        void stop() override {
            std::visit([](auto &&impl) {impl.stop();}, 
                      *static_cast<variant *>(this));
        }
};




purge_agent create_purge_agent(message_queue_subscriber &mq_sub, 
                               bool dry_run=false);

#endif // __PURGE_AGENTS_H__
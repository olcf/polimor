/****************************************************************************
 * Copyright 2023 UT Battelle, LLC
 *
 * This work was supported by the Oak Ridge Leadership Computing Facility at
 * the Oak Ridge National Laboratory, which is managed by UT Battelle, LLC for
 * the U.S. DOE (under the contract No. DE-AC05-00OR22725).
 *
 * This file is part of the PoliMOR project.
 ****************************************************************************/

#ifndef __POLICY_ENGINE_H__
#define __POLICY_ENGINE_H__

#include "../agents/agents.h"
#include "../messaging/messaging.h"

class policy_engine : public agent {
    public:
        policy_engine() = default;
        policy_engine(policy_engine &&) = default;
        policy_engine(const policy_engine &) = default;
        policy_engine &operator=(policy_engine &&) = default;
        policy_engine &operator=(const policy_engine &) = default;
        virtual ~policy_engine() = default;
};

policy_engine *create_policy_engine(const message_queue_subscriber &scan_mq_sub, 
                                    const message_queue_publisher &removal_mq_pub,
                                    const message_queue_publisher &migration_mq_pub,
                                    const message_queue_publisher &recorder_mq_pub);



#endif // __POLICY_ENGINE_H__
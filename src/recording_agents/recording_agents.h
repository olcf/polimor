/****************************************************************************
 * Copyright 2023 UT Battelle, LLC
 *
 * This work was supported by the Oak Ridge Leadership Computing Facility at
 * the Oak Ridge National Laboratory, which is managed by UT Battelle, LLC for
 * the U.S. DOE (under the contract No. DE-AC05-00OR22725).
 *
 * This file is part of the PoliMOR project.
 ****************************************************************************/

#ifndef __RECORDING_AGENTS_H__
#define __RECORDING_AGENTS_H__

#include "../agents/agents.h"
#include "../messaging/messaging.h"

enum class RECORDING_AGENTS {
    SQLITE,
};

class recording_agent : public agent {

    public:
        recording_agent() = default;
        virtual ~recording_agent() = default;
};


template<RECORDING_AGENTS>
recording_agent *create_recording_agent(MsgService auto &ms, 
                                        MsgSubscriber auto &mq_sub);

#endif // __RECORDING_AGENTS_H__
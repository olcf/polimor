/****************************************************************************
 * Copyright 2023 UT Battelle, LLC
 *
 * This work was supported by the Oak Ridge Leadership Computing Facility at
 * the Oak Ridge National Laboratory, which is managed by UT Battelle, LLC for
 * the U.S. DOE (under the contract No. DE-AC05-00OR22725).
 *
 * This file is part of the PoliMOR project.
 ****************************************************************************/

#include "purge_agents.h"

purge_agent create_purge_agent(message_queue_subscriber &mq_sub, 
                               bool dry_run) {
     return { purge_agent_impl(mq_sub, dry_run) };
}
/****************************************************************************
 * Copyright 2023 UT Battelle, LLC
 *
 * This work was supported by the Oak Ridge Leadership Computing Facility at
 * the Oak Ridge National Laboratory, which is managed by UT Battelle, LLC for
 * the U.S. DOE (under the contract No. DE-AC05-00OR22725).
 *
 * This file is part of the PoliMOR project.
 ****************************************************************************/

#include "./migration_agent.h"

template<>
migration_agent create_migration_agent<migration_agents::LFS_MIGRATE>(
    message_queue_subscriber &mq_sub, bool dry_run) {

    return { std::move(lfs_migrate_migration_agent_impl(mq_sub, dry_run)) };
}
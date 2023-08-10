/****************************************************************************
 * Copyright 2023 UT Battelle, LLC
 *
 * This work was supported by the Oak Ridge Leadership Computing Facility at
 * the Oak Ridge National Laboratory, which is managed by UT Battelle, LLC for
 * the U.S. DOE (under the contract No. DE-AC05-00OR22725).
 *
 * This file is part of the PoliMOR project.
 ****************************************************************************/

#include "./scan_agent.h"


/* Template specialization for creating the lfs_find scanner implemented in 
 * this file */
template<> 
scan_agent create_scan_agent<scan_agents::LFS_FIND>(
                    const message_queue_publisher &mq_publisher, 
                    std::string_view path,
                    std::chrono::seconds scan_interval) {

    return scan_agent(lfs_find_scan_agent_impl(mq_publisher, path, scan_interval));
}
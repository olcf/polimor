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

#include <string>
#include "../../messaging/messaging.h"


class lfs_find_scan_agent_impl {

    private:
        message_queue_publisher _mq_pub;
        std::string _executable;
        std::string _path;
        std::chrono::seconds _scan_interval;
        bool _stop;
        

    public:

        /* TODO Need to change this from cat to lfs_find */
        lfs_find_scan_agent_impl(
                            const message_queue_publisher &mq_pub, 
                            std::string_view path,
                            std::chrono::seconds scan_interval=std::chrono::seconds{30},
                            std::string_view executable="/usr/bin/lfs") : 
            _mq_pub(mq_pub), _path(path), _scan_interval(scan_interval), _executable(executable), _stop(false) {} // _pid(-1), _stop(false), _pipefds{-1, -1}  {}


        lfs_find_scan_agent_impl(const lfs_find_scan_agent_impl &) = delete;
        lfs_find_scan_agent_impl &operator=(const lfs_find_scan_agent_impl &) = delete;

        lfs_find_scan_agent_impl(lfs_find_scan_agent_impl &&o) :
            _mq_pub(o._mq_pub), 
            _executable(std::move(o._executable)), 
            _path(std::move(o._path)),
            _scan_interval(std::move(o._scan_interval)),
            _stop(o._stop) {

        }

        lfs_find_scan_agent_impl &operator=(lfs_find_scan_agent_impl &&rhs) {
            _mq_pub = rhs._mq_pub;
            _executable = std::move(rhs._executable);
            _path = std::move(rhs._path);
            _scan_interval = std::move(rhs._scan_interval);
            _stop = rhs._stop;
        
            return *this;
        }

        ~lfs_find_scan_agent_impl() = default;

        void run();

        void stop() {
            this->_stop = true;
        }
};



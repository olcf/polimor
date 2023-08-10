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

#include <concepts>

#include "./messages.h"


/* All supported messaging_service services must be listed here */
enum class messaging_services { 
    POSIX,
    JETSTREAM,
};



template<typename MsgPublisherImpl>
concept MsgPublisher = requires(MsgPublisherImpl impl,
                                const scan_message &scan_msg,
                                const purge_message &purge_msg,
                                const migration_message &migration_msg,
                                const recorder_message &record_msg) {

    requires IsMsg<scan_message>;
    requires IsMsg<purge_message>;
    requires IsMsg<migration_message>;
    requires IsMsg<recorder_message>;

    { impl.template send<scan_message>(scan_msg)  } -> std::same_as<void>;
    { impl.template send<purge_message>(purge_msg) } -> std::same_as<void>;
    { impl.template send<migration_message>(migration_msg) } -> std::same_as<void>;
    { impl.template send<recorder_message>(record_msg) } -> std::same_as<void>;
};



template<typename MsgSubscriberImpl>
concept MsgSubscriber = requires(MsgSubscriberImpl impl) {
       
    requires IsMsg<scan_message>;
    requires IsMsg<purge_message>;

    {  impl.template receive<scan_message>() } -> std::same_as<scan_message>;
    {  impl.template receive<purge_message>() } -> std::same_as<purge_message>;
    {  impl.template receive<migration_message>() } -> std::same_as<migration_message>;
    {  impl.template receive<recorder_message>() } -> std::same_as<recorder_message>;
};

template<typename MsgServiceImpl> 
concept MsgService = requires(MsgServiceImpl impl) {

    { impl.get_type() } -> std::same_as<messaging_services>;
    { impl.create_queue_publisher(std::initializer_list<std::string_view> {}) } -> MsgPublisher;
    { impl.create_queue_subscriber(std::initializer_list<std::string_view> {}) } -> MsgSubscriber;

  
};



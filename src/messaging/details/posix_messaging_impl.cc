/****************************************************************************
 * Copyright 2023 UT Battelle, LLC
 *
 * This work was supported by the Oak Ridge Leadership Computing Facility at
 * the Oak Ridge National Laboratory, which is managed by UT Battelle, LLC for
 * the U.S. DOE (under the contract No. DE-AC05-00OR22725).
 *
 * This file is part of the PoliMOR project.
 ****************************************************************************/

#include <iostream>
#include <string_view>
#include <tuple>
#include <utility>
#include <system_error>
#include <cerrno>

#include "posix_messaging_impl.h"

auto posix_messaging_service_impl::create_queue_publisher(
    const std::string_view name) -> posix_message_queue_publisher_impl {

    struct mq_attr attr = { .mq_flags   = 0, 
                    .mq_maxmsg  = posix_messaging_service_impl::max_msgs, 
                    .mq_msgsize = posix_messaging_service_impl::max_msgsize, 
                    .mq_curmsgs = 0 };


    char url[name.size() + 2]; 
    
    url[0] = '/';
    auto [in, out] = std::ranges::copy(name, url+1);
    *out = '\0';


    /* TODO Add the ability to specify exact number of messages and message size */
    mqd_t mpd = mq_open(url, O_RDWR|O_CREAT, S_IRWXU, &attr);

    if(mpd == (mqd_t) -1) {
        throw std::system_error(errno, std::generic_category());
    }

    return posix_message_queue_publisher_impl(mpd);
}


auto posix_messaging_service_impl::create_queue_subscriber(
    const std::string_view name) -> posix_message_queue_subscriber_impl {

    struct mq_attr attr = { .mq_flags   = 0, 
                    .mq_maxmsg  = posix_messaging_service_impl::max_msgs, 
                    .mq_msgsize = posix_messaging_service_impl::max_msgsize, 
                    .mq_curmsgs = 0 };
    

    /* Fix the mq name with a preceeding '/' */
    char url[name.size() + 2];
    
    url[0] = '/';
    auto [in, out] = std::ranges::copy(name, url+1);
    *out = '\0';

    
    /* TODO Add the ability to specify exact number of messages and message size */
    mqd_t mpd = mq_open(url, O_RDWR|O_CREAT, S_IRWXU, &attr);

    if(mpd == (mqd_t) -1) {
        throw std::system_error(errno, std::generic_category());
    }

    return posix_message_queue_subscriber_impl(mpd);
}

// template<>
// auto create_messaging_service<messaging_services::POSIX>() -> posix_messaging_service_impl {
//     return posix_messaging_service_impl();
// }

/****************************************************************************
 * Copyright 2023 UT Battelle, LLC
 *
 * This work was supported by the Oak Ridge Leadership Computing Facility at
 * the Oak Ridge National Laboratory, which is managed by UT Battelle, LLC for
 * the U.S. DOE (under the contract No. DE-AC05-00OR22725).
 *
 * This file is part of the PoliMOR project.
 ****************************************************************************/


#include "./messaging.h"

template<>
messaging_service create_messaging_service<messaging_services::POSIX>() {
    return { posix_messaging_service_impl() };
}

template<>
messaging_service create_messaging_service<messaging_services::JETSTREAM>(
    std::string_view urls) {

    /* Hidden function declaration for private impl */
    jetstream_messaging_service_impl _create_messaging_service(std::string_view urls);

    return { _create_messaging_service(urls) };
}

template<>
messaging_service create_messaging_service<messaging_services::JETSTREAM>() {

    return { create_messaging_service<messaging_services::JETSTREAM>(
        std::string_view(NATS_DEFAULT_URL)) };
}
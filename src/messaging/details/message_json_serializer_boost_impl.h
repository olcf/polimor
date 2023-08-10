/****************************************************************************
 * Copyright 2023 UT Battelle, LLC
 *
 * This work was supported by the Oak Ridge Leadership Computing Facility at
 * the Oak Ridge National Laboratory, which is managed by UT Battelle, LLC for
 * the U.S. DOE (under the contract No. DE-AC05-00OR22725).
 *
 * This file is part of the PoliMOR project.
 ****************************************************************************/

#ifndef __JSON_SERIALIZER_IMPL_BOOST_IMPL_H__
#define __JSON_SERIALIZER_IMPL_BOOST_IMPL_H__

#include "./serializers.h"


template<typename MSG>
class json_serializer_impl {

    public:

        explicit json_serializer_impl() = default;

        json_serializer_impl(const json_serializer_impl &) = delete;
        json_serializer_impl &operator=(const json_serializer_impl &) = delete;

        json_serializer_impl(json_serializer_impl &&) = default;
        json_serializer_impl &operator=(json_serializer_impl &&) = default;

        std::string_view operator()(const MSG &msg, const std::string_view buffer) const;
        std::string operator()(const MSG &msg) const;
};


#endif // __JSON_SERIALIZER_IMPL_BOOST_IMPL_H__
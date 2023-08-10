/****************************************************************************
 * Copyright 2023 UT Battelle, LLC
 *
 * This work was supported by the Oak Ridge Leadership Computing Facility at
 * the Oak Ridge National Laboratory, which is managed by UT Battelle, LLC for
 * the U.S. DOE (under the contract No. DE-AC05-00OR22725).
 *
 * This file is part of the PoliMOR project.
 ****************************************************************************/

#ifndef __SERIALIZERS_H__
#define __SERIALIZERS_H__

#include <string>
#include <string_view>
#include <concepts>

#include "./messages.h"

/* TODO may need to move these else where and make a concept */


template<template<typename> typename MsgSerializerImpl, typename MSG>
concept MsgSerializerLike = requires(MsgSerializerImpl<MSG> serializer, 
                                     const MSG &msg, 
                                     std::string_view sv) {
        { serializer.operator()(msg, sv) } -> std::same_as<std::string_view>;          
        { serializer.operator()(msg) } -> std::same_as<std::string>;
    } and
    IsMsg<MSG>;

template<template<typename> typename MsgDeserializerImpl, typename MSG>
concept MsgDeserializerLike = requires(MsgDeserializerImpl<MSG> deserializer,
                                       std::string_view sv) {

        { deserializer.operator()(sv) } -> std::same_as<MSG>;
    } and
    IsMsg<MSG>;

template<template<typename> typename MsgSerializerImpl, typename MSG> 
    requires MsgSerializerLike<MsgSerializerImpl, MSG>
struct serializer : MsgSerializerImpl<MSG> { };


template<template<typename> typename MsgDeserializerImpl, typename MSG>
    requires MsgDeserializerLike<MsgDeserializerImpl, MSG>
struct deserializer : MsgDeserializerImpl<MSG> { };


#endif // __SERIALIZERS_H__
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

#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>

#include "./messaging_common.h"
#include "./message_json_serializer_boost_impl.h"
#include "./message_json_deserializer_boost_impl.h"

class posix_message_queue_publisher_impl {

    friend class posix_messaging_service_impl;

    private:
        mqd_t _mqd = -1;

    public:

        posix_message_queue_publisher_impl() = delete;
        posix_message_queue_publisher_impl(mqd_t mqd) : _mqd(mqd) { }

        posix_message_queue_publisher_impl(const posix_message_queue_publisher_impl &) = delete;

        posix_message_queue_publisher_impl(posix_message_queue_publisher_impl &&other) {
            std::swap(this->_mqd, other._mqd);
        }

        posix_message_queue_publisher_impl &operator=(const posix_message_queue_publisher_impl &) = delete;

        posix_message_queue_publisher_impl &operator=(posix_message_queue_publisher_impl &&other) {
            std::swap(this->_mqd, other._mqd);
            return *this;
        }


         ~posix_message_queue_publisher_impl() {
            mq_close(std::exchange(this->_mqd, -1));
        }

        template<typename MSG, 
                 template<typename> typename SERIALIZER=json_serializer_impl> 
            requires IsMsg<MSG> and MsgSerializerLike<SERIALIZER, MSG>
        void send(const MSG &msg) {

            /* C++11 promises this will safely initialized in a multithreaded
             * environment */
            /* TODO ensure this not object sliced and that buffer is large 
             * enough and aligned properly */
            static SERIALIZER<MSG> serializer;
            char buffer[8192];

            auto sv = serializer(msg, { buffer, sizeof(buffer) } );

            int rc = mq_send(this->_mqd, sv.data(), sv.size(), 1);

            switch(rc) {
                case -1:
                    throw std::system_error(errno, std::generic_category());
            }
        }
};

class posix_message_queue_subscriber_impl {

    friend class posix_messaging_service_impl;

    private:
        mqd_t _mqd = -1;

    public:

        posix_message_queue_subscriber_impl() = delete;
        posix_message_queue_subscriber_impl(mqd_t mqd) : _mqd(mqd) { }

        posix_message_queue_subscriber_impl(const posix_message_queue_subscriber_impl &) = delete;

        posix_message_queue_subscriber_impl(posix_message_queue_subscriber_impl &&other) {
            std::swap(this->_mqd, other._mqd);
        }

        posix_message_queue_subscriber_impl &operator=(const posix_message_queue_subscriber_impl &) = delete;

        posix_message_queue_subscriber_impl &operator=(posix_message_queue_subscriber_impl &&other) {
            std::swap(this->_mqd, other._mqd);

            return *this;
        }


         ~posix_message_queue_subscriber_impl() {
            mq_close(std::exchange(this->_mqd, -1));
        }

        template<typename MSG, 
                 template<typename> typename DESERIALIZER=json_deserializer_impl>
            requires IsMsg<MSG> and 
                     MsgDeserializerLike<DESERIALIZER, MSG> and 
                     std::default_initializable<MSG>
        MSG receive() {

            /* C++11 promises this will safely initialized in a multithreaded
             * environment */
            /* TODO ensure this not object sliced */
            static DESERIALIZER<MSG> _deserializer; 
            
            /* TODO page align this. find maximum message size and queue size */
            char buf[8192];


            ssize_t msg_size = mq_receive(this->_mqd, buf, sizeof(buf), NULL);

            switch(msg_size) {

                case -1:
                    throw std::system_error(errno, std::generic_category());

                default:
                    return _deserializer( { 
                        buf, 
                        static_cast<std::string_view::size_type>(msg_size) } );
            }
       }
};


class posix_messaging_service_impl {

    public:

        /* TODO need to figure out the maximuim number of messages and size */
        static constexpr size_t max_msgsize = 8*1024*sizeof(char);
        static constexpr int max_msgs = 10;

        explicit posix_messaging_service_impl() = default;

        posix_messaging_service_impl(const posix_messaging_service_impl &) = delete;

        posix_messaging_service_impl(posix_messaging_service_impl &&other) = default;

        posix_messaging_service_impl &operator=(const posix_messaging_service_impl &) = delete;

        posix_messaging_service_impl &operator=(posix_messaging_service_impl &&other) = default;

        ~posix_messaging_service_impl() = default;

        messaging_services get_type() {
            return messaging_services::POSIX;
        }

        auto create_queue_publisher(std::string_view name) -> posix_message_queue_publisher_impl;
        
        auto create_queue_publisher(std::initializer_list<std::string_view> l) -> posix_message_queue_publisher_impl {
            
            [[unlikely]]
            if(l.size() != 1) {
                throw std::runtime_error("Invalid number of arguments for creating posix publisher");
            } 

            auto v = std::data(l);
            return create_queue_publisher(v[0]); 
        }

        auto create_queue_publisher(const std::tuple<std::string_view> &l) -> posix_message_queue_publisher_impl {
            
            // [[unlikely]]
            // if(l.size() != 1) {
            //     throw std::runtime_error("Invalid number of arguments for creating posix publisher");
            // } 

            // auto v = std::data(l);
            return create_queue_publisher(std::get<0>(l)); 
        }
        
        auto create_queue_subscriber(std::string_view name) -> posix_message_queue_subscriber_impl;

        auto create_queue_subscriber(std::initializer_list<std::string_view> l) -> posix_message_queue_subscriber_impl {

            [[unlikely]]
            if(l.size() != 1) {
                throw std::runtime_error("Invalid number of arguments for creating posxi subscriber");
            } 

            auto v = std::data(l);
            return create_queue_subscriber(v[0]); 
        }
};


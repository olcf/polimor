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


#include <initializer_list>
#include <memory>

#include <nats/nats.h>

#include "./messaging_common.h"
#include "./serializers.h"
#include "./message_json_deserializer_boost_impl.h"
#include "./message_json_serializer_boost_impl.h"


/* Aliases for shared_ptr types */
using shared_natsConnection_ptr_t = std::shared_ptr<natsConnection>;// decltype(&natsConnection_Destroy)>;
using shared_jsCtx_ptr_t = std::shared_ptr<jsCtx>; //, decltype(&jsCtx_Destroy)>;
using shared_natsSubscription_ptr = std::shared_ptr<natsSubscription>;

constexpr size_t _find_buffer_size_for_number(uint64_t number) {

    size_t count;

    for(count=0; number > 0; number /= 10, ++count);

    return count + 1;
}

class jetstream_message_queue_publisher_impl {

    friend class jetstream_messaging_service_impl;

    private:
        std::string _stream_name;
        std::string _consumer_name;
        std::string _subject;    
        shared_natsConnection_ptr_t _conn_ptr;
        shared_jsCtx_ptr_t _jsCtx_ptr;
        jsPubOptions    _jsPubOpts;
        std::string _client_id;

        std::atomic<std::uint64_t> _msg_id;
  
        constexpr static std::size_t UINT64_BUFSIZE = _find_buffer_size_for_number(UINT64_MAX);// std::ceil(std::log(UINT64_MAX)) + 1;

        void _send(std::string_view);

    public:
        explicit jetstream_message_queue_publisher_impl() = delete;


        jetstream_message_queue_publisher_impl(std::string_view stream_name,
                                               std::string_view consumer_name,
                                               std::string_view subject, 
                                               shared_natsConnection_ptr_t conn_ptr,
                                               shared_jsCtx_ptr_t jsctx_ptr,
                                               std::string client_id): 
            _stream_name(stream_name),
            _consumer_name(consumer_name),
            _subject(subject), 
            _conn_ptr(std::move(conn_ptr)), 
            _jsCtx_ptr(std::move(jsctx_ptr)),
            _client_id(std::move(client_id)) { 

            jsPubOptions_Init(&_jsPubOpts);
            _jsPubOpts.MaxWait = 30000;
            _jsPubOpts.MsgId = nullptr;

            _msg_id = 0;
        };

        jetstream_message_queue_publisher_impl(const jetstream_message_queue_publisher_impl &) = delete;
        
        jetstream_message_queue_publisher_impl(jetstream_message_queue_publisher_impl &&o) {

            _stream_name   = std::move(o._stream_name);
            _consumer_name = std::move(o._consumer_name);
            _subject       = std::move(o._subject);
            _conn_ptr      = std::move(o._conn_ptr);
            _jsCtx_ptr     = std::move(o._jsCtx_ptr);
            _client_id     = std::move(o._client_id);
            _jsPubOpts     = o._jsPubOpts;
            _msg_id        = o._msg_id.exchange(0);
        };

        jetstream_message_queue_publisher_impl &operator=(const jetstream_message_queue_publisher_impl &) = delete;
        jetstream_message_queue_publisher_impl &operator=(jetstream_message_queue_publisher_impl &&rhs) {

            _stream_name   = std::move(rhs._stream_name);
            _consumer_name = std::move(rhs._consumer_name);
            _subject       = std::move(rhs._subject);
            _conn_ptr      = std::move(rhs._conn_ptr);
            _jsCtx_ptr     = std::move(rhs._jsCtx_ptr);
            _client_id     = std::move(rhs._client_id);
            _jsPubOpts     = rhs._jsPubOpts;
            _msg_id        = rhs._msg_id.exchange(0);

            return *this;
        }

        ~jetstream_message_queue_publisher_impl() = default;

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

            _send(sv);
        }
};


class jetstream_message_queue_subscriber_impl {

    friend class jetstream_messaging_service_impl;

    private:
        std::string _stream_name;
        std::string _consumer_name;
        std::string _subject;    
        shared_natsConnection_ptr_t _conn_ptr = nullptr;
        shared_natsSubscription_ptr _sub_ptr  = nullptr;

        /* Aliases */
        using unique_natsMsgList_ptr_t = std::unique_ptr<natsMsgList, decltype(&natsMsgList_Destroy)>;

        void _receive(const unique_natsMsgList_ptr_t &);

    public:
        jetstream_message_queue_subscriber_impl() = delete;

        jetstream_message_queue_subscriber_impl(
            std::string_view stream_name,
            std::string_view consumer_name, 
            std::string_view subject,
            shared_natsConnection_ptr_t conn_ptr,
            shared_natsSubscription_ptr sub_ptr): 
            _stream_name(stream_name), 
            _consumer_name(consumer_name), 
            _subject(subject), 
            _conn_ptr(std::move(conn_ptr)), 
            _sub_ptr(std::move(sub_ptr)) { };

        jetstream_message_queue_subscriber_impl(const jetstream_message_queue_subscriber_impl &) = delete;
        jetstream_message_queue_subscriber_impl(jetstream_message_queue_subscriber_impl &&) = default;
        jetstream_message_queue_subscriber_impl &operator=(const jetstream_message_queue_subscriber_impl &) = delete;
        jetstream_message_queue_subscriber_impl &operator=(jetstream_message_queue_subscriber_impl &&) = default;

        ~jetstream_message_queue_subscriber_impl() = default;


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
            
            natsMsgList msgList = { nullptr, 0 };
            
            unique_natsMsgList_ptr_t msgListPtr(&msgList, 
                                                natsMsgList_Destroy);
    

            _receive(msgListPtr);

            assert(sizeof(std::string_view::size_type) >= sizeof(int));

            return _deserializer({ natsMsg_GetData(msgList.Msgs[0]), 
                                   static_cast<std::string_view::size_type>(natsMsg_GetDataLength(msgList.Msgs[0])) }); 
        }
};



class jetstream_messaging_service_impl {
    
    
    private:   

        /* TODO need to use smart pointers to track either conn or 
         * the messaging service */
        shared_natsConnection_ptr_t _conn_ptr = nullptr;
        shared_jsCtx_ptr_t _jsCtx_ptr= nullptr;

    public:

        jetstream_messaging_service_impl() = delete;

        
        jetstream_messaging_service_impl(shared_natsConnection_ptr_t conn_ptr, shared_jsCtx_ptr_t jsctx_ptr): 
            _conn_ptr(std::move(conn_ptr)), _jsCtx_ptr(std::move(jsctx_ptr)) { }

        jetstream_messaging_service_impl(const jetstream_messaging_service_impl &) = delete;
        jetstream_messaging_service_impl(jetstream_messaging_service_impl &&) = default;

        jetstream_messaging_service_impl &operator=(const jetstream_messaging_service_impl &) = delete;
        jetstream_messaging_service_impl &operator=(jetstream_messaging_service_impl &&) = default;
            

        virtual ~jetstream_messaging_service_impl() {
        
        }

        messaging_services get_type() {
            return messaging_services::JETSTREAM;
        }

        /* TODO see if we can put the concept constraint back on the left */
        auto create_queue_publisher(std::string_view stream, std::string_view consumer, std::string_view subject) -> jetstream_message_queue_publisher_impl; 
        
        
        auto create_queue_publisher(std::initializer_list<std::string_view> l) -> jetstream_message_queue_publisher_impl {
            
            [[unlikely]]
            if(l.size() != 3) {
                throw std::runtime_error("Invalid number of arguments for creating jetstream publisher");
            } 

            auto v = std::data(l);
            
            return create_queue_publisher(v[0], v[1], v[2]); 
        }

        auto create_queue_subscriber(std::string_view stream, std::string_view consumer, std::string_view subject) -> jetstream_message_queue_subscriber_impl;


        auto create_queue_subscriber(std::initializer_list<std::string_view> l) -> jetstream_message_queue_subscriber_impl {

            [[unlikely]]
            if(l.size() != 3) {
                throw std::runtime_error("Invalid number of arguments for creating jetstream subscriber");
            } 

            auto v = std::data(l);

            return create_queue_subscriber(v[0], v[1], v[2]); 
        }
};



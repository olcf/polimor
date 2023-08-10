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
#include <cstdlib>
#include <exception>
#include <string_view>

#include <mqueue.h>
#include <cassert>

#include "../messaging/details/messaging_common.h"
#include "../messaging/details/message_json_serializer_boost_impl.h"
#include "../messaging/details/message_json_deserializer_boost_impl.h"

#include "../common/qs_exception.h"

struct simple_message : public message_tag {

    std::string payload;

    simple_message() = default;
    simple_message(const char *msg): payload(msg) { };      
};

template<>
std::string_view 
json_serializer_impl<simple_message>::operator()(
        const simple_message &msg, 
        const std::string_view buffer) const {

    /* Write the data to a buffer */
    int rc = snprintf(const_cast<char *>(buffer.data()),
                      buffer.size(), 
                      "{ "
                        "\"payload\": \"%s\" "
                      "}",
                      msg.payload.c_str());

    /* Error serializing the message */
    if(rc < 0) {
    
        throw std::system_error(errno, 
                                std::generic_category(), 
                          "Error serializing message");

    /* Buffer was too small */
    } else if(rc >= buffer.size()) {
        throw std::system_error(ENOMEM, 
                                std::generic_category(), 
                          "Output buffer too small");
        
    }
    
    /* Return the filled buffer */
    return { buffer.data(), static_cast<std::string_view::size_type>(rc + 1)};
}

template<>
void json_deserializer_impl<simple_message>::_validate_message(const simple_message &msg) const { 
    
    if(msg.payload.empty()) {
        throw qs_exception("Error deserializing simple message: msg is invalid");
    }
}

using string_view = boost::core::string_view;
using value_handler = json_deserializer_impl<simple_message>::handler::value_handler;
using object_handler = json_deserializer_impl<simple_message>::handler::object_handler;

template<>
const value_handler json_deserializer_impl<simple_message>::handler::_starting_handler =
    std::move(object_handler { {
        {
            std::string("payload"),
            value_handler([](string_view value, simple_message &msg) {
                msg.payload = value;
            })
        }
    }});

#include "../messaging/messaging.h"


int main(void) {

    mq_unlink("/test_queue");

    try {
        MsgService auto ms = create_messaging_service<messaging_services::POSIX>();

        MsgSubscriber auto queue_sub = ms.create_queue_subscriber(std::string_view("test_queue"));
        MsgPublisher auto queue_pub = ms.create_queue_publisher(std::string_view("test_queue"));


      

        simple_message send_msg( "Hello World");

        queue_pub.send<simple_message>(send_msg);

        auto recv_msg= queue_sub.receive<simple_message>();

        assert(send_msg.payload.compare(recv_msg.payload) == 0);

    }catch(const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

     mq_unlink("/test_queue");

    return EXIT_SUCCESS;
}
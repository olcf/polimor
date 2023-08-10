/****************************************************************************
 * Copyright 2023 UT Battelle, LLC
 *
 * This work was supported by the Oak Ridge Leadership Computing Facility at
 * the Oak Ridge National Laboratory, which is managed by UT Battelle, LLC for
 * the U.S. DOE (under the contract No. DE-AC05-00OR22725).
 *
 * This file is part of the PoliMOR project.
 ****************************************************************************/

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string_view>

#include "../messaging/messaging.h"

constexpr std::string_view queue_name = "test_queue";

struct simple_message : public message {

    std::string payload;
    
    simple_message() = default;
    simple_message(std::string msg): payload(msg) { };
    
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
        throw qs_exception("Error deserializing simple message: payload is invalid");
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


int main(int argc, const char* argv[]) {
    
    
    messaging_service ms = create_messaging_service<messaging_services::NATS>();

    message_queue_publisher mq_pub = ms.create_queue_publisher(queue_name);
    message_queue_subscriber mq_sub = ms.create_queue_subscriber(queue_name);


    for(int i=0; i<1000; i++) {
        simple_message send_msg("hello there " + std::to_string(i));

        mq_pub.send(send_msg);


        auto recv_msg = mq_sub.receive<simple_message>();
        

        std::clog << "Message = " << recv_msg.payload << std::endl;
    }


    return EXIT_SUCCESS;
}
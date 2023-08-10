
#add_library(messaging posix_messaging.cc nats_messaging_impl.cc jetstream_messaging_impl.cc ../messaging.h)
add_library(messaging_impl OBJECT  
                      posix_messaging_impl.cc 
                      jetstream_messaging_impl.cc 
                      scan_message_json_deserializer_boost_impl.cc
                      purge_message_json_deserializer_boost_impl.cc
                      migration_message_json_deserializer_boost_impl.cc
                      recorder_message_json_deserializer_boost_impl.cc
                      scan_message_json_serializer_boost_impl.cc
                      purge_message_json_serializer_boost_impl.cc
                      migration_message_json_serializer_boost_impl.cc
                      recorder_message_json_serializer_boost_impl.cc)
                      
target_link_libraries(messaging_impl PUBLIC Boost::json)
target_link_libraries(messaging_impl PUBLIC rt ${LIBNATS_LDFLAGS})

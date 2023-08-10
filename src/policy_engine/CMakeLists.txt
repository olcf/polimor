
add_executable(policy_engine_cmd policy_engine_cmd.cc)
target_include_directories(policy_engine_cmd PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(policy_engine_cmd policy_engine 
                                        messaging
                                        messaging_impl
                                        config_parser_objs
                                        Boost::program_options)

add_subdirectory("details")


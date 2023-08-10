add_library(scan_agent OBJECT scan_agent.cc)
target_link_libraries(scan_agent PUBLIC scan_agent_impl)

add_executable(scan_agent_cmd scan_agent_cmd.cc)
target_include_directories(scan_agent_cmd PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(scan_agent_cmd scan_agent 
                                     scan_agent_impl
                                     messaging
                                     messaging_impl
                                     config_parser_objs
                                     Boost::program_options
                                     nats)


add_subdirectory("details")

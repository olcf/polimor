add_subdirectory("details")

add_library(purge_agents OBJECT purge_agents.cc)
target_link_libraries(purge_agents PUBLIC purge_agents_impl)

add_executable(purge_agent_cmd purge_agent_cmd.cc)
target_include_directories(purge_agent_cmd PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(purge_agent_cmd PUBLIC purge_agents 
                                             purge_agents_impl
                                              messaging 
                                              messaging_impl 
                                              config_parser_objs
                                              Boost::program_options)



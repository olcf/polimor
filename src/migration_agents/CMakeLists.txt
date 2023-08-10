add_subdirectory(details)

add_library(migration_agents OBJECT migration_agent.cc)
target_link_libraries(migration_agents migration_agents_impl)

add_executable(migration_agent_cmd migration_agent_cmd.cc)
target_include_directories(migration_agent_cmd PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(migration_agent_cmd migration_agents 
                                          messaging 
                                          messaging_impl
                                          config_parser_objs
                                          Boost::program_options)


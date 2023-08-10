add_subdirectory(details)

add_executable(recording_agent_cmd recording_agent_cmd.cc)
target_link_libraries(recording_agent_cmd recording_agents 
                                          messaging 
                                          config_parser_objs
                                          Boost::program_options 
                                          ${OPENSSL_CRYPTO_LIBRARIES} 
                                          ${LIBNATS_LDFLAGS})
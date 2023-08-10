add_executable(posix_messaging_test posix_messaging_test.cc)
target_include_directories(posix_messaging_test PUBLIC ${CMAKE_SOURCE_DIR}/messaging)
target_link_libraries(posix_messaging_test messaging messaging_impl)


add_executable(scan_test scan_test.cc)
target_include_directories(scan_test PUBLIC ${CMAKE_SOURCE_DIR}/scan_agent)
target_link_libraries(scan_test scan_agent scan_agent_impl messaging messaging_impl)

add_executable(regex_test regex_test.cc)


add_executable(policy_engine_test policy_engine_test.cc)
target_include_directories(policy_engine_test PUBLIC ${CMAKE_SOURCE_DIR}/messaging)
target_link_libraries(policy_engine_test policy_engine messaging messaging_impl)

add_executable(process_control_test process_control_test.cc)
target_include_directories(process_control_test PUBLIC ${CMAKE_SOURCE_DIR}/lib)

add_executable(purge_agent_test purge_agent_test.cc)
target_include_directories(purge_agent_test PUBLIC ${CMAKE_SOURCE_DIR}/messaging ${CMAKE_SOURCE_DIR}/lib)
target_link_libraries(purge_agent_test purge_agents purge_agents_impl messaging messaging_impl)

add_executable(migration_agent_test migration_agent_test.cc)
target_include_directories(migration_agent_test PUBLIC ${CMAKE_SOURCE_DIR}/messaging ${CMAKE_SOURCE_DIR}/lib)
target_link_libraries(migration_agent_test migration_agents messaging messaging_impl)

# add_executable(recorder_agent_test recorder_agent_test.cc)
# target_include_directories(recorder_agent_test PUBLIC ${CMAKE_SOURCE_DIR}/messaging ${CMAKE_SOURCE_DIR}/lib)
# target_link_libraries(recorder_agent_test recording_agents messaging ${OPENSSL_CRYPTO_LIBRARIES})

add_executable(concept_test concept_test.cc)

add_executable(jetstream_test jetstream_test.cc)
target_include_directories(jetstream_test PUBLIC ${LIBNATS_INCLUDE_DIRS})
target_link_libraries(jetstream_test messaging messaging_impl ${LIBNATS_LDFLAGS})

add_executable(coroutines_test coroutines_test.cc)
target_include_directories(coroutines_test PUBLIC ${LIBNATS_INCLUDE_DIRS})
target_link_libraries(coroutines_test messaging messaging_impl ${LIBNATS_LDFLAGS})

add_executable(fake_lfs_find fake_lfs_find.cc)

add_executable(pimpl_test pimpl_test.cc)

add_executable(json_deserializer_test json_deserializer_test.cc)
target_include_directories(json_deserializer_test PUBLIC ${CMAKE_SOURCE_DIR}/messaging)
target_link_libraries(json_deserializer_test  messaging messaging_impl)

add_executable(qs_exception_test qs_exception_test.cc)

add_executable(config_parser_test config_parser_test.cc)
target_include_directories(config_parser_test PUBLIC ${CMAKE_SOURCE_DIR}/src/common)
target_link_libraries(config_parser_test PUBLIC config_parser_objs)

add_test(posix_messaging_test1 posix_messaging_test)
add_test(simple_scan_test scan_test 1)
add_test(json_scan_test scan_test 2)
add_test(regex_test1 regex_test)
add_test(policy_engine_test1 policy_engine_test)
add_test(process_control_test1 process_con trol_test)
add_test(purge_agent_test1 purge_agent_test)
add_test(migration_agent_test1 migration_agent_test)
# add_test(recorder_agent_test1 recorder_agent_test)

add_test(concept_test1 concept_test)
add_test(jetstream_test1 jetstream_test)

add_test(coroutines_test1 coroutines_test)


add_test(pimpl_test1 pimpl_test)
add_test(json_deserializer_test1 json_deserializer_test)

add_test(qs_message_test1 qs_message_test)
add_test(config_parser_test1 config_parser_test)
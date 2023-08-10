add_library(recording_agents sqlite_recording_agent_impl.cc)

add_executable(sqlite_recorder_cmd sqlite_recorder_cmd.cc)
target_include_directories(sqlite_recorder_cmd PUBLIC 
        ${SQLite3_INCLUDE_DIRS} 
        ${OPENSSL_INCLUDE_DIR})
target_link_libraries(sqlite_recorder_cmd 
        messaging 
        ${SQLite3_LIBRARIES}
        ${OPENSSL_CRYPTO_LIBRARIES}
        Boost::program_options)

 
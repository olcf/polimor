add_library(scan_agent_impl OBJECT lfs_find_scan_agent.cc)
target_link_libraries(scan_agent_impl INTERFACE messaging)
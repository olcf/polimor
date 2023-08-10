add_subdirectory(details)

add_library(messaging OBJECT messaging.cc)
target_link_libraries(messaging PUBLIC messaging_impl)


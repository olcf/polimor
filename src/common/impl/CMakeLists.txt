
message(${CMAKE_CURRENT_SOURCE_DIR}/../../../contrib/schema.json)
add_custom_command(OUTPUT schema.inc
                   COMMAND hexdump -ve '1/1 \"0x%02x, \"' ${CMAKE_CURRENT_SOURCE_DIR}/../../../contrib/schema.json > ${CMAKE_CURRENT_SOURCE_DIR}/schema.inc
                   DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/../../../contrib/schema.json"
                   COMMENT "Creating schema.inc from schema.json")


add_custom_target(schema_file DEPENDS schema.inc)

add_library(config_parser_objs OBJECT yaml_config.cc)
target_include_directories(config_parser_objs PUBLIC 
                                    ${yaml-cpp_INCLUDE_DIR} 
                                    ${valijson_INCLUDE_DIR})
target_link_libraries(config_parser_objs PRIVATE  ${yaml-cpp_LIBRARIES})

add_dependencies(config_parser_objs schema_file) 
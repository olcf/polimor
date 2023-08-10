cmake_minimum_required(VERSION 3.12)

# Project
project("Lustre Tiering" LANGUAGES CXX VERSION 0.1)


set(ENV{PKG_CONFIG_PATH} "$ENV{PKG_CONFIG_PATH}:/usr/local/lib64/pkgconfig")

set(OPENSSL_USE_STATIC_LIBS False)
set(CMAKE_FIND_STATIC FIRST)

# Adds the build directory to the prefix path so conan cmake files can be found 
# when hunting for packages.
set(CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR} ${CMAKE_MODULE_PATH})
set(CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR} ${CMAKE_PREFIX_PATH})


find_package(SQLite3 REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(YAML-CPP REQUIRED)
find_package(Valijson REQUIRED)
find_package(Boost 1.75.0 REQUIRED COMPONENTS program_options json)
pkg_check_modules(LIBNATS REQUIRED libnats)

# Specify the C++ standard to use
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS YES)
set(CMAKE_CXX_STANDARD_REQUIRED ON)



set(CMAKE_EXPORT_COMPILE_COMMANDS True)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcoroutines-ts")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcoroutines -fconcepts-diagnostics-depth=10")
endif()

enable_testing()

add_subdirectory(src)
add_subdirectory(contrib)


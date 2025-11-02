cmake_minimum_required(VERSION 3.31)
project(sync_service)

set(CMAKE_CXX_STANDARD 23)

option(SYNC_SERVICE_ENABLE_TESTS "Build and run unit tests" ON)

list(PREPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

find_package(Poco REQUIRED COMPONENTS Foundation Util Data Net JWT)
find_package(ReflectCpp REQUIRED)
find_package(stduuid CONFIG REQUIRED)
find_package(ctre CONFIG REQUIRED)
find_package(unofficial-sodium CONFIG REQUIRED)
add_library(sodium::sodium ALIAS unofficial-sodium::sodium)

if(${SYNC_SERVICE_ENABLE_TESTS})
    find_package(GTest CONFIG REQUIRED)
endif()

add_subdirectory(src)
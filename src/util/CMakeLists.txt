project(sync_service_util)

file(GLOB_RECURSE HDR "${PROJECT_SOURCE_DIR}/include/*.h")
file(GLOB_RECURSE SRC "${PROJECT_SOURCE_DIR}/src/*.cpp")

add_library(${PROJECT_NAME} STATIC ${HDR} ${SRC})

target_link_libraries(${PROJECT_NAME}
        PRIVATE
        Poco::Foundation
        Poco::Util
        Poco::Data
        Poco::Net
)

target_include_directories(${PROJECT_NAME}
        PUBLIC
        "${PROJECT_SOURCE_DIR}/include"
)
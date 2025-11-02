project(sync_service_app)

# Lib solution
file(GLOB_RECURSE HDR "${PROJECT_SOURCE_DIR}/include/*.h")
file(GLOB_RECURSE SRC "${PROJECT_SOURCE_DIR}/src/*.cpp")
list(REMOVE_ITEM SRC "${PROJECT_SOURCE_DIR}/src/main.cpp")

add_library(${PROJECT_NAME}_lib ${HDR} ${SRC})

target_link_libraries(${PROJECT_NAME}_lib
        PUBLIC
        sync_service_util

        reflectcpp::reflectcpp
        Poco::Foundation
        Poco::Util
        Poco::Data
        Poco::Net

        PRIVATE
        ctre::ctre
        Poco::JWT
)

target_include_directories(${PROJECT_NAME}_lib
        PUBLIC
        "${PROJECT_SOURCE_DIR}/include"
)

# Final executable solution
add_executable(${PROJECT_NAME} "${PROJECT_SOURCE_DIR}/src/main.cpp")
target_link_libraries(${PROJECT_NAME}
        PUBLIC
        ${PROJECT_NAME}_lib
)

if(SYNC_SERVICE_ENABLE_TESTS)
    include(CTest)
    enable_testing()

    file(GLOB_RECURSE TEST_SRC
            "${PROJECT_SOURCE_DIR}/tests/*.tests.cpp"
    )
    file(GLOB_RECURSE TEST_MOCKS
            "${PROJECT_SOURCE_DIR}/tests/*.mocks.h"
    )

    add_executable(${PROJECT_NAME}_tests ${TEST_MOCKS} ${TEST_SRC})

    target_include_directories(${PROJECT_NAME}_tests
            PUBLIC
            "${PROJECT_SOURCE_DIR}/include"
            "${PROJECT_SOURCE_DIR}/tests"
    )

    target_link_libraries(${PROJECT_NAME}_tests
            PRIVATE
            ${PROJECT_NAME}_lib
            sync_service_util

            GTest::gtest
            GTest::gtest_main
            GTest::gmock
            Poco::Foundation
            Poco::Util
            Poco::Data
            Poco::Net
    )

    add_test(
            NAME sync_service_tests
            COMMAND ${PROJECT_NAME}_tests
    )

endif()

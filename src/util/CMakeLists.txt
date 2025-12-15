project(sync_service_util)

file(GLOB_RECURSE HDR "${PROJECT_SOURCE_DIR}/include/*.h")
file(GLOB_RECURSE SRC "${PROJECT_SOURCE_DIR}/src/*.cpp")

add_library(${PROJECT_NAME} STATIC ${HDR} ${SRC})

target_link_libraries(${PROJECT_NAME}
        PRIVATE

        reflectcpp::reflectcpp
        sodium::sodium
        Poco::Foundation
        Poco::Util
        Poco::Data
        Poco::Net
)

target_include_directories(${PROJECT_NAME}
        PUBLIC
        "${PROJECT_SOURCE_DIR}/include"
)

if(SYNC_SERVICE_ENABLE_TESTS)
    include(CTest)
    enable_testing()

    file(GLOB_RECURSE TEST_SRC
            "${PROJECT_SOURCE_DIR}/tests/*.tests.cpp"
    )

    add_executable(${PROJECT_NAME}_tests ${TEST_SRC})

    target_include_directories(${PROJECT_NAME}_tests
            PUBLIC
            "${PROJECT_SOURCE_DIR}/include"
            "${PROJECT_SOURCE_DIR}/tests"
    )

    target_link_libraries(${PROJECT_NAME}_tests
            PRIVATE
            ${PROJECT_NAME}

            GTest::gtest
            GTest::gtest_main
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

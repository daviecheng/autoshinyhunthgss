add_executable(capture_tests
    capture_enums_test.cpp
    screen_locator_test.cpp
)

target_include_directories(capture_tests
    ## White box testing for now
    PRIVATE
        ${CAPTURE_ROOT_DIR}/private_include
)

target_compile_definitions(capture_tests
    PRIVATE
        CAPTURE_TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/test_data"
)

target_link_libraries(capture_tests
    PRIVATE
        capture
        GTest::gtest_main
        PkgConfig::OpenCV
)

include(GoogleTest)

gtest_discover_tests(capture_tests)

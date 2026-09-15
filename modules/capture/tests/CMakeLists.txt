add_executable(capture_tests
    capture_enums_test.cpp
)

target_link_libraries(capture_tests
    PRIVATE
        capture
        GTest::gtest_main
)

include(GoogleTest)

gtest_discover_tests(capture_tests)

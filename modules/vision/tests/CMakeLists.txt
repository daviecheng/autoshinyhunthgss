add_executable(vision_tests
    vision_enums_test.cpp
)

target_link_libraries(vision_tests
    PRIVATE
        vision
        GTest::gtest_main
)

include(GoogleTest)

gtest_discover_tests(vision_tests)
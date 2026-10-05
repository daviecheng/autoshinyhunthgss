add_executable(vision_tests
    band_mask_test.cpp
    frame_view_test.cpp
    vision_config_test.cpp
    vision_enums_test.cpp
)

target_include_directories(vision_tests
    ## White box testing for now
    PRIVATE
        ${VISION_ROOT_DIR}/private_include
)

target_link_libraries(vision_tests
    PRIVATE
        vision
        GTest::gtest_main
        PkgConfig::OpenCV
)

include(GoogleTest)

gtest_discover_tests(vision_tests)
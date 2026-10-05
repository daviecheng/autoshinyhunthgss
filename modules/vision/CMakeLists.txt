add_library(vision STATIC)

target_sources(vision
    PRIVATE
        # Add src files here
        src/band_mask.cpp
        src/frame_view.cpp
        src/screen_classifier.cpp
        src/vision_config.cpp
)

target_include_directories(vision
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/private_include
)

target_link_libraries(vision
    PUBLIC
        capture
    PRIVATE
        PkgConfig::OpenCV
)

target_compile_features(vision
    PUBLIC
        cxx_std_17
)

set(VISION_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

if(BUILD_TESTING AND BUILD_VISION_TESTS)
    add_subdirectory(tests)
endif()
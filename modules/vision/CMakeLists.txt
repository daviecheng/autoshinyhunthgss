add_library(vision STATIC)

target_sources(vision
    PRIVATE
        # Add src files here
        src/screen_classifier.cpp
)

target_include_directories(vision
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/private_include
)

target_compile_features(vision
    PUBLIC
        cxx_std_17
)

if(BUILD_TESTING AND BUILD_VISION_TESTS)
    add_subdirectory(tests)
endif()
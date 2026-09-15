add_library(capture STATIC)

target_sources(capture
    PRIVATE
        # Add src files here
        src/frame_source.cpp
)

target_include_directories(capture
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/private_include
)

target_compile_features(capture
    PUBLIC
        cxx_std_17
)

if(BUILD_TESTING AND BUILD_CAPTURE_TESTS)
    add_subdirectory(tests)
endif()

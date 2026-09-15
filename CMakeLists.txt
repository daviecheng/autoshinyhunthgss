cmake_minimum_required(VERSION 3.10)

project(
    autoshinyhunthgss
    LANGUAGES CXX
)

include(CTest)

option(BUILD_CAPTURE_TESTS "Build capture tests" ON)
option(BUILD_VISION_TESTS "Build vision tests" ON)

message (STATUS "BUILD_TESTING = ${BUILD_TESTING}")
message (STATUS "BUILD_CAPTURE_TESTS = ${BUILD_CAPTURE_TESTS}")
message (STATUS "BUILD_VISION_TESTS = ${BUILD_VISION_TESTS}")

if(BUILD_TESTING)
    find_package(GTest REQUIRED)
endif()

add_subdirectory(modules/capture)
add_subdirectory(modules/vision)
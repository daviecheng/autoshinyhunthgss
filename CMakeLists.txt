cmake_minimum_required(VERSION 3.10)

project(
    autoshinyhunthgss
    LANGUAGES CXX
)

include(CTest)

# OpenCV is located through pkg-config rather than find_package. Both the
# development machine and the Raspberry Pi are Linux, and opencv4.pc is correct
# on both, whereas some distributions ship a broken OpenCVConfig.cmake. This
# ties the build to Linux -- Windows would need find_package instead.
find_package(PkgConfig REQUIRED)
pkg_check_modules(OpenCV REQUIRED IMPORTED_TARGET opencv4)
message (STATUS "OpenCV_VERSION = ${OpenCV_VERSION}")

# Builds the camera source when libcamera is installed, and skips it otherwise.
# -DBUILD_WITH_LIBCAMERA=OFF skips it even when installed
option(BUILD_WITH_LIBCAMERA "Build the libcamera frame source when libcamera is installed" ON)

if(BUILD_WITH_LIBCAMERA)
    pkg_check_modules(LIBCAMERA IMPORTED_TARGET libcamera)

    if(NOT LIBCAMERA_FOUND)
        message(WARNING "libcamera not found; building without the camera frame source")
    endif()
endif()
message(STATUS "BUILD_WITH_LIBCAMERA = ${BUILD_WITH_LIBCAMERA} (found: ${LIBCAMERA_FOUND})")

# Build unit test options
option(BUILD_CAPTURE_TESTS "Build capture tests" ON)
option(BUILD_VISION_TESTS "Build vision tests" ON)

message (STATUS "BUILD_TESTING = ${BUILD_TESTING}")
message (STATUS "BUILD_CAPTURE_TESTS = ${BUILD_CAPTURE_TESTS}")
message (STATUS "BUILD_VISION_TESTS = ${BUILD_VISION_TESTS}")

if(BUILD_TESTING)
    find_package(GTest REQUIRED)
endif()

# Project modules
add_subdirectory(modules/app)
add_subdirectory(modules/capture)
add_subdirectory(modules/vision)
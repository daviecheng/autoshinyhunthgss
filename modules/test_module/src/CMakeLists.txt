cmake_minimum_required(VERSION 3.9)

set(${PROJECT_NAME}_PUBLIC_INCLUDES
    ${PROJECT_SOURCE_DIR}/include
)

set(${PROJECT_NAME}_PRIVATE_INCLUDES
    ${PROJECT_SOURCE_DIR}/private_include
)

set(${PROJECT_NAME}_SRC
    private_sample.cpp
    sample.cpp
)

add_library(${PROJECT_NAME} STATIC)

target_sources(
    ${PROJECT_NAME}
    PRIVATE ${${PROJECT_NAME}_SRC}
)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
        $<INSTALL_INTERFACE:${${PROJECT_NAME}_PUBLIC_INCLUDES}>
        $<BUILD_INTERFACE:${${PROJECT_NAME}_PUBLIC_INCLUDES}>
    PRIVATE
        ${${PROJECT_NAME}_PRIVATE_INCLUDES}
)

target_compile_features(
    ${PROJECT_NAME}
    PUBLIC cxx_std_17
)
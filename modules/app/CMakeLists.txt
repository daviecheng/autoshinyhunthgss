add_executable(soft_reset_hunt
    src/main.cpp
    src/soft_reset_hunt.cpp
)

target_link_libraries(soft_reset_hunt
    PRIVATE
        capture
)

target_compile_features(soft_reset_hunt
    PRIVATE
        cxx_std_17
)

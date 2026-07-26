add_library(
    inferlite_core
    STATIC
    core/version.cpp
)

target_include_directories(
    inferlite_core
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)
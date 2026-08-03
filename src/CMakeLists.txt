add_library(
    inferlite_core
    STATIC
    core/inference_io.cpp
    core/status.cpp
    core/tensor.cpp
    core/version.cpp
)

target_link_libraries(
    inferlite_core
    PRIVATE
        inferlite_project_options
        inferlite_sanitizers
)

target_include_directories(
    inferlite_core
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

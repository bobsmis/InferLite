add_library(
    inferlite_core
    STATIC
    core/inference_io.cpp
    core/status.cpp
    core/tensor.cpp
    core/version.cpp
    backend/fake_backend.cpp
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

add_library(
    inferlite_http
    STATIC
    http/http_server.cpp
)

target_include_directories(
    inferlite_http
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(
    inferlite_http
    PUBLIC
        inferlite_core
    PRIVATE
        httplib::httplib
        inferlite_project_options
        inferlite_sanitizers
)
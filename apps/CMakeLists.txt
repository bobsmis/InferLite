add_executable(
    inferlite_demo
    demo_main.cpp
)

target_link_libraries(
    inferlite_demo
    PRIVATE
        inferlite_core
        inferlite_project_options
        inferlite_sanitizers
)

add_executable(
    inferlite_server
    server_main.cpp
)

target_link_libraries(
    inferlite_server
    PRIVATE
        inferlite_http
        inferlite_project_options
        inferlite_sanitizers
)
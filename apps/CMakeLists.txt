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

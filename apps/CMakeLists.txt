
add_executable(#编译可执行文件
    inferlite_demo#编译的可执行文件名
    demo_main.cpp#参与编译的源文件
)

target_link_libraries(#为可执行文件链接静态库
    inferlite_demo#绑定的文件名
    PRIVATE
        inferlite_core
        inferlite_project_options
         inferlite_sanitizers
)
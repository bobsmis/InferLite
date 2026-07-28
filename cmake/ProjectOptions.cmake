add_library(inferlite_project_options INTERFACE)#创建一个interface目标

target_compile_features(     #配置绑定到这个 INTERFACE 目标。后续任何库 / 可执行文件只要通过target_link_libraries链接它，就会自动继承这条 C++ 版本要求。
    inferlite_project_options
    INTERFACE  #写INTERFACE：只有链接当前目标的外部代码会生效
        cxx_std_17  #代码必须使用 C++17 标准编译
)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(
        inferlite_project_options
        INTERFACE
            -Wall
            -Wextra
            -Wpedantic
            -Wconversion
            -Wsign-conversion
    )
elseif(MSVC)
    target_compile_options(
        inferlite_project_options
        INTERFACE
            /W4
    )
endif()

option(#option(变量名 "说明文字" 默认值)
    INFERLITE_WARNINGS_AS_ERRORS
    "将所有编译警告看作编译错误"
    OFF
)

if(INFERLITE_WARNINGS_AS_ERRORS)
    if(MSVC)
        target_compile_options(
            inferlite_project_options
            INTERFACE
                /WX
        )
    else()
        target_compile_options(
            inferlite_project_options
            INTERFACE
                -Werror
        )
    endif()
endif()
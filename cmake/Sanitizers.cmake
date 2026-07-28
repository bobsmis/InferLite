add_library(inferlite_sanitizers INTERFACE)

option(
    INFERLITE_ENABLE_SANITIZERS
    "启用地址消毒器和未定义行为消毒器"
    OFF
)

if(INFERLITE_ENABLE_SANITIZERS)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(
            inferlite_sanitizers
            INTERFACE
                -fsanitize=address,undefined
                -fno-omit-frame-pointer
        )

        target_link_options(
            inferlite_sanitizers
            INTERFACE
                -fsanitize=address,undefined
                -fno-omit-frame-pointer
        )
    else()
        message(
            WARNING
            "Sanitizers are only configured for GCC and Clang"
        )
    endif()
endif()
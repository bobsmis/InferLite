add_library(inferlite_project_options INTERFACE)

target_compile_features(
    inferlite_project_options
    INTERFACE
        cxx_std_17
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

option(
    INFERLITE_WARNINGS_AS_ERRORS
    "Treat InferLite warnings as errors"
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
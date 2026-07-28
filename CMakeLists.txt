cmake_minimum_required(VERSION 3.20)

project(
    InferLite
    VERSION 0.0.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
include(cmake/Sanitizers.cmake)
include(cmake/ProjectOptions.cmake)

add_subdirectory(src)
add_subdirectory(apps)

include(CTest)

#这一步的含义：启用CTest测试支持；
#创建BUILD_TESTING选项，默认通常为ON。

if(BUILD_TESTING)
    include(cmake/Dependencies.cmake)
    add_subdirectory(tests)
endif()
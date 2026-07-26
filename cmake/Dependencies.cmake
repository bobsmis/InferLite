include(FetchContent)

FetchContent_Declare(
    googletest  #引入第三方模块googletest
    GIT_REPOSITORY https://github.com/google/googletest.git #  指定googletest的git仓库地址
    GIT_TAG v1.17.0
)
#引入第三方模块，方便下载和使用第三方库



set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)#  

FetchContent_MakeAvailable(googletest)
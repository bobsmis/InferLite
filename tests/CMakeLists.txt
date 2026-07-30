add_executable(
    inferlite_tests
    unit/status_test.cpp
    unit/version_test.cpp
    unit/tensor_test.cpp
)

target_link_libraries(
    inferlite_tests
    PRIVATE
        inferlite_core
        inferlite_project_options
         inferlite_sanitizers
        GTest::gtest_main
)

include(GoogleTest)

gtest_discover_tests(inferlite_tests)
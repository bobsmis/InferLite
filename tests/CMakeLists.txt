add_executable(
    inferlite_tests
    unit/inference_io_test.cpp
    unit/status_test.cpp
    unit/status_or_test.cpp
    unit/tensor_test.cpp
    unit/version_test.cpp
)

target_link_libraries(
    inferlite_tests
    PRIVATE
        GTest::gtest_main
        inferlite_core
        inferlite_project_options
        inferlite_sanitizers
)

include(GoogleTest)

gtest_discover_tests(inferlite_tests)

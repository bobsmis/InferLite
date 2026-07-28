add_executable(
    inferlite_tests
    unit/version_test.cpp
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
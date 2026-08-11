add_executable(
    inferlite_tests
    unit/inference_io_test.cpp
    unit/status_test.cpp
    unit/status_or_test.cpp
    unit/tensor_test.cpp
    unit/version_test.cpp
    unit/fake_backend_test.cpp
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

add_test(
    NAME InferLiteCliTest
    COMMAND
        ${CMAKE_COMMAND}
        -DDEMO_PATH=$<TARGET_FILE:inferlite_demo>
        -P ${CMAKE_CURRENT_SOURCE_DIR}/integration/cli_test.cmake
)

if(NOT DEFINED DEMO_PATH)
    message(FATAL_ERROR "DEMO_PATH is required")
endif()

function(run_cli_case case_name expected_exit expected_stdout expected_stderr)
    execute_process(
        COMMAND "${DEMO_PATH}" ${ARGN}
        RESULT_VARIABLE actual_exit
        OUTPUT_VARIABLE actual_stdout
        ERROR_VARIABLE actual_stderr
    )

    if(NOT "${actual_exit}" STREQUAL "${expected_exit}")
        message(
            FATAL_ERROR
            "${case_name}: expected exit ${expected_exit}, got ${actual_exit}"
        )
    endif()

    if(NOT "${actual_stdout}" STREQUAL "${expected_stdout}")
        message(
            FATAL_ERROR
            "${case_name}: unexpected stdout\nexpected: [${expected_stdout}]\nactual: [${actual_stdout}]"
        )
    endif()

    if(NOT "${actual_stderr}" STREQUAL "${expected_stderr}")
        message(
            FATAL_ERROR
            "${case_name}: unexpected stderr\nexpected: [${expected_stderr}]\nactual: [${actual_stderr}]"
        )
    endif()
endfunction()

run_cli_case(
    success
    0
    "model=demo_model\noutput name=x shape=[2] data=[1,2]\n"
    ""
)

run_cli_case(
    forced_failure
    1
    ""
    "inference failed: fake backend forced failure\n"
    --fail
)

run_cli_case(
    unknown_argument
    2
    ""
    "usage: inferlite_demo [--fail]\n"
    --unknown
)

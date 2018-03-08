# Additional analysis to perform clang-format/clang-tidy
# Get all project files
file(GLOB_RECURSE
        ALL_CXX_SOURCE_FILES
        irohad/*.[ch]pp
        iroha-cli/*.[ch]pp
        libs/*.[ch]pp
        shared_model/*.[ch]pp
        test/*.[ch]pp
        )

# Adding clang-format target if executable is found
if(NOT CLANG_FORMAT_BIN)
    find_program(CLANG_FORMAT_BIN "clang-format")
endif()

if(CLANG_FORMAT_BIN)
    message(STATUS "Target clang-format enabled")
    add_custom_target(
            clang-format
            COMMAND "${CLANG_FORMAT_BIN}"
            -i
            ${ALL_CXX_SOURCE_FILES}
    )
endif()

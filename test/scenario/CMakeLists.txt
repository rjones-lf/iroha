########################################################################################
# Executable java_query_repo_invocation_test
########################################################################################
add_executable(virtual_currency_test virtual_currency_test.cpp)
target_link_libraries(virtual_currency_test
    Repository
    executor
    gtest
)
add_test(
    NAME virtual_currency_test
    COMMAND $<TARGET_FILE:virtual_currency_test>
)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/test_bin)

########################################################################################
# Executable Virtual currency test
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

########################################################################################
# Executable tax test
########################################################################################
add_executable(transfer_with_tax_test transfer_with_tax_test.cpp)
target_link_libraries(transfer_with_tax_test
    Repository
    executor
    gtest
)
add_test(
    NAME transfer_with_tax_test
    COMMAND $<TARGET_FILE:transfer_with_tax_test>
)
set(CMAKE_BUILD_TYPE Debug)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/test_bin)

addtest(stateless_transaction_validator_test stateless/transaction_validator_test.cpp)
target_link_libraries(stateless_transaction_validator_test
        stateless_validator
        model
        hash
        crypto
        )
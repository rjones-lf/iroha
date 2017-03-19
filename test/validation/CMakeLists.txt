# Grpc connection Test
add_executable(transaction_validator_test
        transaction_validator_test.cpp
        )
target_link_libraries(transaction_validator_test
        signature
        config_manager
        peer_service
        gtest
        validator
        connection_with_grpc
        event_with_grpc
        )
add_test(
        NAME transaction_validator_test
        COMMAND $<TARGET_FILE:transaction_validator_test>
)

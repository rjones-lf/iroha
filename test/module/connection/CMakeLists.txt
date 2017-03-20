# Grpc connection Test
add_executable(connection_grpc_test
        connection_with_grpc_test.cpp
)
target_link_libraries(connection_grpc_test
    connection_with_grpc
    config_manager
    peer_service
    transaction_builder
    gtest
)
add_test(
    NAME connection_grpc_test
    COMMAND $<TARGET_FILE:connection_grpc_test>
)

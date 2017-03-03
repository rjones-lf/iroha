# Grpc connection Test
add_executable(connection_grpc_test
    connection_with_grpc_test.cpp
)
target_link_libraries(connection_grpc_test
    connection_with_grpc
    signature
    config_manager
    ed25519
    gtest
)
add_test(
    NAME connection_grpc_test
    COMMAND $<TARGET_FILE:connection_grpc_test>
)

add_executable(sumeragi_test
    sumeragi_test.cpp
)

target_link_libraries(sumeragi_test
    leveldb
    base64
    signature
    sumeragi
    connection_with_grpc
    config_manager
)

add_executable(transaction_test
    transaction_test.cpp
)

target_link_libraries(transaction_test
    base64
    signature
    sumeragi
    connection_with_grpc
    config_manager
)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/my_test_bin)

SET(LEVELDB_PATH "${PROJECT_SOURCE_DIR}/core/vendor/leveldb")

add_executable(sumeragi_test
    sumeragi_test.cpp
)


target_link_libraries(sumeragi_test
    leveldb
    snappy
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
    snappy
    base64
    signature
    sumeragi
    connection_with_grpc
    config_manager
)

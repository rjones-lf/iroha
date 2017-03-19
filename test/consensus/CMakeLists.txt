# leave this as is, these tests should be in separate directory!
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/my_test_bin)

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
        peer_service
        transaction_builder
        )

add_executable(transaction_test
        transaction_test.cpp
        )

target_link_libraries(transaction_test
        base64
        signature
        sumeragi
        connection_with_grpc
        pthread
        config_manager
        peer_service
        transaction_builder
        )

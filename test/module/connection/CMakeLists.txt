add_executable(connection_with_grpc_flatbuffer_test
        connection_with_grpc_flatbuffer_test.cpp
        )
target_link_libraries(connection_with_grpc_flatbuffer_test
        endpoint_fbs
        config_manager
        flatbuffer_service
        connection_with_grpc_flatbuffer
        gtest
        )
add_test(
        NAME connection_with_grpc_flatbuffer_test
        COMMAND $<TARGET_FILE:connection_with_grpc_flatbuffer_test>
)

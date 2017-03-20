add_executable(ensure_sumeragi_json_format_test ensure_sumeragi_json_format.cpp)
target_link_libraries(ensure_sumeragi_json_format_test
    config_manager
    peer_service
    json
    gtest
)
add_test(
    NAME ensure_sumeragi_json_format_test
    COMMAND $<TARGET_FILE:ensure_sumeragi_json_format_test>
)


add_executable(peer_service_with_json_test peer_service_with_json_test.cpp)
target_link_libraries(peer_service_with_json_test
    config_manager
    peer_service
    pthread
    connection_with_grpc
    signature
    config_manager
    peer_service
    event_with_grpc
    gtest
    transaction_builder
    transaction_repository
)
add_test(
    NAME peer_service_with_json_test
    COMMAND $<TARGET_FILE:peer_service_with_json_test>
)

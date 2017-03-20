add_executable(ensure_sumeragi_json_format_test ensure_sumeragi_json_format.cpp)
target_link_libraries(ensure_sumeragi_json_format_test
    config_manager
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
    pthread
    connection_with_grpc
    signature
    config_manager
    event_with_grpc
    gtest
    create_objects_helper
    transaction_repository
)
add_test(
    NAME peer_service_with_json_test
    COMMAND $<TARGET_FILE:peer_service_with_json_test>
)

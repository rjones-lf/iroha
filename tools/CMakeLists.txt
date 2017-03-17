SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

execute_process(COMMAND bash "-c" "cd $IROHA_HOME/tools/helper; ./allbuild.sh")

# make config
add_executable(make_sumeragi make_sumeragi )
target_link_libraries(make_sumeragi
    base64
    signature
    json
)

add_executable(issue_transaction issue_transaction.cpp )
target_link_libraries(issue_transaction
    json
    config_manager
    logger
    event_with_grpc
    create_objects_helper
    connection_with_grpc
    transaction_repository
)

# make config
add_executable(send_izanami_test send_izanami.cpp)
target_link_libraries(send_izanami_test
    json
    config_manager
    logger
    event_with_grpc
    create_objects_helper
    connection_with_grpc
    transaction_repository
)
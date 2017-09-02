set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

# Gflags config validators
add_library(cli-flags_validators
    validators.cpp
    )
target_link_libraries(cli-flags_validators
    gflags
    )

add_library(keys_manager
    impl/keys_manager_impl.cpp
    )
target_link_libraries(keys_manager
    optional
    crypto
    )

add_library(client
    client.cpp
    impl/query_response_handler.cpp
    impl/transaction_response_handler.cpp
    impl/grpc_response_handler.cpp
    )
target_link_libraries(client
    pb_model_converters
    json_model_converters
    model_generators
    command_client
    query_client
    logger
    )

target_include_directories(client PUBLIC
    ${PROJECT_SOURCE_DIR}/iroha-cli
    )

# IrohaCli
add_executable(iroha-cli
    main.cpp
    validators.cpp
    )
target_link_libraries(iroha-cli
    client
    cli-flags_validators
    keys_manager
    )

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

add_subdirectory(interactive)

# Gflags config validators
add_library(cli-flags_validators
    validators.cpp
    )
target_link_libraries(cli-flags_validators
    gflags
    )

add_library(client
    client.cpp
    impl/query_response_handler.cpp
    impl/transaction_response_handler.cpp
    impl/grpc_response_handler.cpp
    )
target_link_libraries(client
    interactive_cli
    crypto
    optional
    logger
    rapidjson
    pb_model_converters
    json_model_converters
    model_generators
    command_client
    query_client
    model_generators
    parser
    model
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

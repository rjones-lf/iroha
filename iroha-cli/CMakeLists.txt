set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

# GenesisBlockClient
add_library(genesis_block_client genesis_block_client_impl.cpp)
target_link_libraries(genesis_block_client
    endpoint
    optional
    model
    )

# BootstrapNetwork
add_library(bootstrap_network bootstrap_network.cpp)
target_link_libraries(bootstrap_network
    genesis_block_client
    rapidjson
    model
    ametsuchi
    ip_tools
    )

# Gflags config validators
add_library(cli-flags_validators validators.cpp)
target_link_libraries(cli-flags_validators gflags)

add_library(keys_manager impl/keys_manager_impl.cpp)
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
    model_converters
    crypto
    optional
    logger
    rapidjson
    command_client
    query_client
    ametsuchi
    model_generators
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
    bootstrap_network
    rapidjson
    ametsuchi
    model
    keys_manager
    )

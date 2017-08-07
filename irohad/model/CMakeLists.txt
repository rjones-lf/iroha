add_library(model
    model_crypto_provider_impl.cpp
    model_hash_provider_impl.cpp
    impl/stateful_command_validation.cpp
    impl/command_execution.cpp
    impl/model_operators.cpp
    converters/impl/pb_block_factory.cpp
    converters/impl/pb_transaction_factory.cpp
    converters/impl/pb_command_factory.cpp
    converters/impl/pb_query_response_factory.cpp
    impl/query_execution.cpp
    converters/impl/pb_query_factory.cpp
    converters/impl/json_common.cpp
    converters/impl/json_command_factory.cpp
    converters/impl/json_transaction_factory.cpp
    converters/impl/json_block_factory.cpp
    )

target_link_libraries(model
    schema
    optional
    rxcpp
    hash
    crypto
    rapidjson
    logger
    )


add_library(model_converters
    converters/impl/json_query_factory.cpp
    )

target_link_libraries(model_converters
    rapidjson
    optional
    schema
    logger
    )

add_library(model_registrations INTERFACE)
target_include_directories(model_registrations INTERFACE
    registration
    )
target_link_libraries(model_registrations INTERFACE
    model
    )

add_subdirectory(generators)

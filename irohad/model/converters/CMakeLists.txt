add_library(json_model_converters
    impl/json_block_factory.cpp
    impl/json_command_factory.cpp
    impl/json_common.cpp
    impl/json_query_factory.cpp
    impl/json_transaction_factory.cpp
    )

target_link_libraries(json_model_converters
    schema
    optional
    rxcpp
    hash
    crypto
    rapidjson
    logger
    model
    )

add_library(pb_model_converters
    impl/pb_block_factory.cpp
    impl/pb_command_factory.cpp
    impl/pb_query_factory.cpp
    impl/pb_query_response_factory.cpp
    impl/pb_transaction_factory.cpp
    )

target_link_libraries(pb_model_converters
    schema
    optional
    rxcpp
    hash
    crypto
    logger
    model
    )

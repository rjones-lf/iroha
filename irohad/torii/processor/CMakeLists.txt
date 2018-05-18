add_library(processors
    impl/transaction_processor_impl.cpp
    impl/query_processor_impl.cpp
    )

target_link_libraries(processors PUBLIC
    rxcpp
    logger
    endpoint
    shared_model_proto_builders
    )

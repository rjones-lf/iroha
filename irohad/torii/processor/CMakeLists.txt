add_library(processors
    impl/transaction_processor_impl.cpp
    impl/query_processor_impl.cpp
    )

target_link_libraries(processors PUBLIC
    rxcpp
    logger
    endpoint
    mst_processor
    shared_model_proto_builders
    query_execution
    )

add_library(client_processor
    impl/transaction_processor_stub.cpp
    impl/stub_query_processor.cpp
    )

target_link_libraries(client_processor PUBLIC
    model
    rxcpp
    )

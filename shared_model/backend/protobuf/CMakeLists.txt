add_library(shared_model_proto_backend
    impl.cpp
    )

target_link_libraries(shared_model_proto_backend
    schema
    model_interfaces
    iroha_amount
    )

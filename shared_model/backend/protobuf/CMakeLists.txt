add_library(shared_model_proto_backend
    impl.cpp
    impl/permissions.cpp
    )

target_link_libraries(shared_model_proto_backend
    schema
    shared_model_interfaces
    iroha_amount
    )

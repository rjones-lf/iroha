addtest(synchronizer_test synchronizer_test.cpp)
target_link_libraries(synchronizer_test
    synchronizer
    shared_model_cryptography
    shared_model_proto_backend
    shared_model_stateless_validation
    shared_model_interfaces_factories
    shared_model_default_builders
    )

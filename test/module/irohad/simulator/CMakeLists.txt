addtest(simulator_test simulator_test.cpp)
target_link_libraries(simulator_test
    simulator
    shared_model_stateless_validation
    shared_model_cryptography_model
    )


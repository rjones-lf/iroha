add_library(stateless_validator
    transaction_validator_stub.cpp
    validator_stub.cpp
    )

target_link_libraries(stateless_validator
    schema
    optional
    ed25519
    rxcpp
    model
    )

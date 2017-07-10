add_library(stateless_validator
    validator_impl.cpp
    )

target_link_libraries(stateless_validator
    schema
    optional
    ed25519
    rxcpp
    model
    )

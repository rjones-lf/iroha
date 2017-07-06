add_library(chain_validator
    block_validator_stub.cpp
    validator_stub.cpp
    )

target_link_libraries(chain_validator
    ametsuchi
    rxcpp
    schema
    optional
    )
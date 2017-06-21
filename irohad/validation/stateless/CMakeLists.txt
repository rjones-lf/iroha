ADD_LIBRARY(stateless_validator STATIC validator.cpp)

target_link_libraries(stateless_validator
        schema
        optional
        keccak
        )
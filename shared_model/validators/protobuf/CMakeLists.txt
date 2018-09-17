add_library(proto_validation
    proto_transaction_validator.cpp
    )

target_link_libraries(proto_validation
    schema
    )

add_library(model
    model.cpp
    model_crypto_provider_stub.cpp
    )

target_link_libraries(model
    schema
    optional
    rxcpp
    )

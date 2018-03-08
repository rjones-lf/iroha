add_library(crypto_provider
    impl/crypto_provider_impl.cpp
    )

target_link_libraries(crypto_provider
    model
    )

add_library(dao
    dao.cpp
    dao_crypto_provider_stub.cpp
    dao_hash_provider_impl.cpp
    dao_crypto_provider_impl.cpp
    )

target_link_libraries(dao
    schema
    optional
    rxcpp
    hash
    crypto
    )

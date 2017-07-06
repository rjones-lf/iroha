add_library(dao
    dao.cpp
    dao_crypto_provider_stub.cpp
    )

target_link_libraries(dao
    schema
    optional
    rxcpp
    )

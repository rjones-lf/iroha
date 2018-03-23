add_library(shared_model_cryptography_model
    blob.cpp
    hash.cpp
    keypair.cpp
    private_key.cpp
    public_key.cpp
    seed.cpp
    signed.cpp
    )

target_link_libraries(shared_model_cryptography_model
    boost
    )

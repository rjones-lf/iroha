add_library(crypto STATIC
    common.hpp
    base64.hpp
    hash.hpp
    keypair.hpp
    crypto.cpp
    crypto.hpp
)

target_link_libraries(crypto PUBLIC
    keccak
    ed25519
)

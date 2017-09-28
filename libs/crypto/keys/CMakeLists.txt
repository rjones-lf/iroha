# currently not used
# but eventually should be
# TODO(@warchant): add pubkey, add keypair implementation

add_library(keypair
    keypair.cpp
    keypair.hpp
    private_key.hpp
    )
target_link_libraries(keypair
    crypto
    )

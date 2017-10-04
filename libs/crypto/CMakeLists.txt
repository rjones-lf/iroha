add_subdirectory(ed25519)

add_library(hash
    hash.cpp
    )

target_link_libraries(hash
    ed25519
    common
    pb_model_converters
    )

add_library(crypto
    ed25519_impl.cpp
    )
target_link_libraries(crypto
    ed25519
    hash
    )

add_library(lookup3
    lookup3.c
    )

add_library(keys_manager
    keys_manager_impl.cpp
    )

target_link_libraries(keys_manager
    optional
    crypto
    )

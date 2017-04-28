
add_library(repository STATIC repository.cpp)

target_link_libraries(repository
    iroha_ametsuchi
    keccak
    flatbuffers
)



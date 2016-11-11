SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/my_test_bin)

SET(CMAKE_CXX_FLAGS_DEBUG "-g")

SET(KECCAK_PATH  "${PROJECT_SOURCE_DIR}/core/vendor/KeccakCodePackage")
SET(ED25519_PATH "${PROJECT_SOURCE_DIR}/core/vendor/ed25519")
SET(AERON_PATH  "${PROJECT_SOURCE_DIR}/core/vendor/Aeron")

link_directories(
    ${PROJECT_BINARY_DIR}/lib
    ${ED25519_PATH}
    ${KECCAK_PATH}/bin/generic64
    ${PROJECT_SOURCE_DIR}/core/infra/crypto
    ${AERON_PATH}/cppbuild/Debug/lib
)

add_executable(sumeragi_test
    sumeragi_test.cpp
)
target_link_libraries(sumeragi_test
    snappy
    aeron_client
    objects
    hash
    base64
    signature
    sumeragi
    connection_with_aeron
    peer_service_with_json
    pthread
)

add_executable(transaction_test
    transaction_test.cpp
)
target_link_libraries(transaction_test
        snappy
        aeron_client
        objects
        hash
        base64
        signature
        sumeragi
        connection_with_aeron
        peer_service_with_json
        pthread
)
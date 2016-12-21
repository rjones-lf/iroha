
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/test_bin)


SET(CRYPTO_PATH  "${PROJECT_SOURCE_DIR}/core/infra/crypto")
SET(ED25519_PATH "${PROJECT_SOURCE_DIR}/core/vendor/ed25519")

link_directories(
    ${CRYPTO_PATH}
    ${ED25519_PATH}/lib
)

# Grpc connection Test
add_executable(connection_grpc_test
  connection_with_grpc_test.cpp
)
target_link_libraries(connection_grpc_test
  convertor
  connection_with_grpc
  consensus_event
  signature
  peer_service_with_json
)
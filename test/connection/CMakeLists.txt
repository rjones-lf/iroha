
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/test_bin)

#[[ 
# Aeron connection Test
SET(AERON_PATH  "${PROJECT_SOURCE_DIR}/core/vendor/Aeron")

include_directories(
  ${AERON_PATH}/aeron-client/src/main/cpp
)

link_directories(
    ${AERON_PATH}/cppbuild/Release/lib
)
add_executable(connection_test
  connection_with_aeron_test.cpp
)
target_link_libraries(connection_test
  aeron_client
  connection_with_aeron
  pthread
)
#]]

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
  connection_with_grpc
  consensus_event
  signature
  peer_service_with_json
)
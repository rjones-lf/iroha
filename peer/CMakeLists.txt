
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

SET(AERON_PATH "${PROJECT_SOURCE_DIR}/core/vendor/Aeron")
SET(LEVELDB_PATH ${PROJECT_SOURCE_DIR}/core/vendor/leveldb)
SET(KECCAK_PATH  "${PROJECT_SOURCE_DIR}/core/vendor/KeccakCodePackage")
SET(ED25519_PATH "${PROJECT_SOURCE_DIR}/core/vendor/ed25519")
SET(JSON_PATH "${PROJECT_SOURCE_DIR}/core/vendor/json")

include_directories(
  ${PROJECT_SOURCE_DIR}/core/server
  ${PROJECT_SOURCE_DIR}/core/peer

  ${LEVELDB_PATH}/include
  ${AERON_PATH}/aeron-client/src/main/cpp
  ${ED25519_PATH}/src
  ${JSON_PATH}/src
)

link_directories(
  ${PROJECT_BINARY_DIR}/lib
  ${ED25519_PATH}
  ${KECCAK_PATH}/bin/generic64
  ${PROJECT_SOURCE_DIR}/core/infra/crypto 
  ${AERON_PATH}/cppbuild/Debug/lib
  ${LEVELDB_PATH}/out-static
)

add_executable(iroha-main
  main.cpp
)
target_link_libraries(iroha-main
  event_repository
  merkle_transaction_repository
  world_state_repo_with_level_db
  sumeragi
  snappy
  logger
  http_server_with_cappuccino
  connection_with_aeron
  peer_service
  pthread
)



SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)


SET(CMAKE_CXX_FLAGS "-g -Wall -std=c++1y")

SET(AERON_PATH "${PROJECT_SOURCE_DIR}/core/vendor/Aeron")
SET(LEVELDB_PATH "${PROJECT_SOURCE_DIR}/core/vendor/leveldb")
SET(KECCAK_PATH  "${PROJECT_SOURCE_DIR}/core/vendor/KeccakCodePackage")
SET(ED25519_PATH "${PROJECT_SOURCE_DIR}/core/vendor/ed25519")
SET(JSON_PATH "${PROJECT_SOURCE_DIR}/core/vendor/json")
SET(THREAD_POOL_PATH "${PROJECT_SOURCE_DIR}/core/vendor/thread_pool_cpp")

include_directories(
  ${PROJECT_SOURCE_DIR}/core/server
  ${PROJECT_SOURCE_DIR}/core/peer
  ${THREAD_POOL_PATH}/thread_pool
  ${LEVELDB_PATH}/include
  ${AERON_PATH}/aeron-client/src/main/cpp
  ${ED25519_PATH}/src
  ${JSON_PATH}/src
)

link_directories(
  ${PROJECT_BINARY_DIR}/lib
  ${ED25519_PATH}/lib
  ${KECCAK_PATH}/bin/generic64
  ${PROJECT_SOURCE_DIR}/core/infra/crypto 
  ${AERON_PATH}/cppbuild/Release/lib
  ${LEVELDB_PATH}/out-static
)

add_executable(iroha-main
  main.cpp
)
target_link_libraries(iroha-main
  # used grpc
  grpc++_reflection
  protobuf
  pthread
  dl

  commands
  leveldb
  snappy
  world_state_repo_with_level_db
  sumeragi
  logger
  http_server_with_cappuccino
  connection_with_grpc
  peer_service_with_json
  pthread
  keccak
  hash
)


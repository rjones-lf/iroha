SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

add_executable(iroha-main
  main.cpp
)
target_link_libraries(iroha-main
  # used grpc
  grpc++_reflection
  protobuf
  dl

  leveldb
  snappy
  world_state_repo_with_level_db
  sumeragi
  logger
  http_server_with_cappuccino
  connection_with_grpc
  config_manager
  ed25519
  hash
  pthread
  thread_pool
  json
)

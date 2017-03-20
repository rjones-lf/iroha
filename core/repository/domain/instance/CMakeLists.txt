SET(CMAKE_CXX_FLAGS "-g -Wall -std=c++1y -fPIC")

SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

add_library(core_repository STATIC
  domain_repository.cpp
  account_repository.cpp
  asset_repository.cpp
  simple_asset_repository.cpp
  peer_repository.cpp
)

target_link_libraries(core_repository
  exception
  # connect infra
  world_state_repo_with_level_db
  event_with_grpc
  protobuf
  leveldb
)

add_library(transaction_repository STATIC
  transaction_repository.cpp
)

target_link_libraries(transaction_repository
  base64
  event_with_grpc # protobuf
  world_state_repo_with_level_db
  signature # consensus/consensus_event.hpp requires (should be fixed?)
)

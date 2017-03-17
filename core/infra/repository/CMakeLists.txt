ADD_LIBRARY(world_state_repo_with_level_db STATIC world_state_repository_with_level_db.cpp)
target_link_libraries(world_state_repo_with_level_db
  snappy
  leveldb
  logger
  config_manager
  exception
  json
)

ADD_LIBRARY(transaction_repository STATIC transaction_repository.cpp)
target_link_libraries(transaction_repository
    world_state_repo_with_level_db
)

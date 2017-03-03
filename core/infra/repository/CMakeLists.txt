ADD_LIBRARY(world_state_repo_with_level_db STATIC world_state_repository_with_level_db.cpp)
target_link_libraries(world_state_repo_with_level_db
  snappy
  leveldb
  logger
  config_manager
  exception
  json
)

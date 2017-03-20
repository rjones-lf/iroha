#####################################
#   world state repo with leveldb   #
#####################################
add_library(world_state_repo_with_level_db STATIC
  world_state_repository_with_level_db.cpp
)

target_link_libraries(world_state_repo_with_level_db
  leveldb
  logger
  config_manager
  exception
)

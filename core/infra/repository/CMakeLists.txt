SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
SET(LEVELDB_PATH ${PROJECT_SOURCE_DIR}/core/vendor/leveldb)

SET(CMAKE_CXX_FLAGS "-g -Wall -std=c++1y -fPIC")

include_directories(
  ${LEVELDB_PATH}/include
  ${LEVELDB_PATH}/include
)

link_directories(
  ${LEVELDB_PATH}/out-shared
  ${LEVELDB_PATH}/out-static
)

ADD_LIBRARY(world_state_repo_with_level_db STATIC world_state_repository_with_level_db.cpp)
target_link_libraries(world_state_repo_with_level_db
  snappy
  leveldb
  logger
  config_manager
  exception
  json
)

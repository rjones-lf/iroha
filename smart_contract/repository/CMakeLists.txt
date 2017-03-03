cmake_minimum_required(VERSION 3.3.0 FATAL_ERROR)

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

include_directories(
  ${LEVELDB_PATH}/include
)

link_directories(
  ${PROJECT_BINARY_DIR}/lib
  ${LEVELDB_PATH}/out-shared
  ${LEVELDB_PATH}/out-static
)

add_library(Repository SHARED
  repository_jni.cpp
)

target_link_libraries(Repository
  leveldb
  core_repository
  virtual_machine_with_java
  event_with_grpc
  create_objects_helper
)

add_custom_command(TARGET Repository
  POST_BUILD
  COMMAND /bin/sh ${PROJECT_SOURCE_DIR}/smart_contract/compile_test.sh
)

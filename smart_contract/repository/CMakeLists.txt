cmake_minimum_required(VERSION 3.3.0 FATAL_ERROR)

add_library(Repository SHARED
  repository_jni.cpp
)

target_link_libraries(Repository
  core_repository
  jvm
)

add_custom_command(TARGET Repository
  POST_BUILD
  COMMAND /bin/sh ${PROJECT_SOURCE_DIR}/smart_contract/compile_test.sh
)

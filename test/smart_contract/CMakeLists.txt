########################################################################################
# Executable java_query_repo_invocation_test
########################################################################################
add_executable(java_query_repo_invocation_test java_query_repo_invocation_test.cpp)
target_link_libraries(java_query_repo_invocation_test
  Repository
  leveldb
  virtual_machine_with_java
  world_state_repo_with_level_db
  jvm
  gtest
  config_manager
)
add_test(
  NAME java_query_repo_invocation_test
  COMMAND $<TARGET_FILE:java_query_repo_invocation_test>
)


########################################################################################
# Executable java_query_repo_account_test
########################################################################################
add_executable(java_query_repo_account_test java_query_repo_account_test.cpp)
target_link_libraries(java_query_repo_account_test
  Repository
  leveldb
  virtual_machine_with_java
  world_state_repo_with_level_db
  jvm
  gtest
)
add_test(
  NAME java_query_repo_account_test
  COMMAND $<TARGET_FILE:java_query_repo_account_test>
)

########################################################################################
# Executable java_query_repo_asset_test
########################################################################################
add_executable(java_query_repo_asset_test java_query_repo_asset_test.cpp)
target_link_libraries(java_query_repo_asset_test
  Repository
  leveldb
  virtual_machine_with_java
  world_state_repo_with_level_db
  jvm
  gtest
)
add_test(
  NAME java_query_repo_asset_test
  COMMAND $<TARGET_FILE:java_query_repo_asset_test>
)

########################################################################################
# Executable java_query_repo_simple_asset_test
########################################################################################
add_executable(java_query_repo_simple_asset_test java_query_repo_simple_asset_test.cpp)

target_link_libraries(java_query_repo_simple_asset_test
  Repository
  leveldb
  virtual_machine_with_java
  world_state_repo_with_level_db
  jvm
  gtest
)
add_test(
  NAME java_query_repo_simple_asset_test
  COMMAND $<TARGET_FILE:java_query_repo_simple_asset_test>
)

########################################################################################
# Executable java_query_repo_domain_test
########################################################################################
add_executable(java_query_repo_domain_test java_query_repo_domain_test.cpp)

target_link_libraries(java_query_repo_domain_test
  Repository
  leveldb
  virtual_machine_with_java
  world_state_repo_with_level_db
  jvm
  gtest
)
add_test(
  NAME java_query_repo_domain_test
  COMMAND $<TARGET_FILE:java_query_repo_domain_test>
)

########################################################################################
# Executable java_query_repo_peer_test
########################################################################################
add_executable(java_query_repo_peer_test java_query_repo_peer_test.cpp)

target_link_libraries(java_query_repo_peer_test
  Repository
  leveldb
  virtual_machine_with_java
  world_state_repo_with_level_db
  jvm
  gtest
)
add_test(
  NAME java_query_repo_peer_test
  COMMAND $<TARGET_FILE:java_query_repo_peer_test>
)

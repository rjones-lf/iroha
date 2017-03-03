add_executable(world_state_repository_with_leveldb_test
    world_state_repository_with_leveldb_test.cpp
)

target_link_libraries(world_state_repository_with_leveldb_test
    leveldb
    snappy
    world_state_repo_with_level_db
    gtest
)

add_test(
    NAME world_state_repository_with_leveldb_test
    COMMAND $<TARGET_FILE:world_state_repository_with_leveldb_test>
)

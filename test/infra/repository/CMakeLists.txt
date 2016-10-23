
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/test_bin)

SET(LEVELDB_PATH ${PROJECT_SOURCE_DIR}/core/vendor/leveldb)

include_directories(
    ${LEVELDB_PATH}/include
    ${GTEST_INCLUDE_DIR}
)
link_directories(
    ${PROJECT_BINARY_DIR}/lib
    ${LEVELDB_PATH}/out-static
)

add_executable(world_state_repository_with_leveldb_test
    world_state_repository_with_leveldb_test.cpp
)
target_link_libraries(world_state_repository_with_leveldb_test
    world_state_repo_with_level_db
    snappy
    leveldb
    ${GTEST_LIBS_DIR}/libgtest.a ${GTEST_LIBS_DIR}/libgtest_main.a pthread)
add_test(
    NAME world_state_repository_with_leveldb_test
    COMMAND $<TARGET_FILE:world_state_repository_with_leveldb_test>
)

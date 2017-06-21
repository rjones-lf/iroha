set(CMAKE_BUILD_TYPE Debug)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/test_bin)

addtest(dao_hash_provider_test dao/dao_hash_provider_test.cpp)
target_link_libraries(dao_hash_provider_test
        dao
        optional
        keccak
        )
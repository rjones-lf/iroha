addtest(ametsuchi_test ametsuchi_test.cpp)
target_link_libraries(ametsuchi_test
    ametsuchi
    libs_common
    ametsuchi_fixture
    )

addtest(wsv_query_command_test wsv_query_command_test.cpp)
target_link_libraries(wsv_query_command_test
    ametsuchi
    libs_common
    )

addtest(block_storage_test block_storage_test.cpp)
target_link_libraries(block_storage_test
    block_storage_nudb
    logger
    )

addtest(integration_block_query_nudb_test integration_block_query_nudb_test.cpp)
target_link_libraries(integration_block_query_nudb_test
    ametsuchi
    )

addtest(block_query_transfer_test block_query_transfer_test.cpp)
target_link_libraries(block_query_transfer_test
    ametsuchi
    libs_common
    ametsuchi_fixture
    )

addtest(kv_storage_test kv_storage_test.cpp)
target_link_libraries(kv_storage_test
    ametsuchi
    libs_common
    )

add_library(ametsuchi_fixture INTERFACE)
target_link_libraries(ametsuchi_fixture INTERFACE
    pqxx
    cpp_redis
    model_generators
    )


addtest(unit_block_query_test unit_block_query_test.cpp)
target_link_libraries(unit_block_query_test
    ametsuchi
    )

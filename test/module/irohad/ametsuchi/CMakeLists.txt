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

addtest(flat_file_test flat_file_test.cpp)
target_link_libraries(flat_file_test
    ametsuchi
    libs_common
    flat_file
    )

addtest(block_query_test block_query_test.cpp)
target_link_libraries(block_query_test
    ametsuchi
    libs_common
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
    model_generators
    )

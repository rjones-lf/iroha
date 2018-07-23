addtest(ametsuchi_test ametsuchi_test.cpp)
target_link_libraries(ametsuchi_test
    ametsuchi
    libs_common
    ametsuchi_fixture
    shared_model_stateless_validation
    )

addtest(wsv_query_command_test wsv_query_command_test.cpp)
target_link_libraries(wsv_query_command_test
    ametsuchi
    libs_common
    ametsuchi_fixture
    )

addtest(flat_file_test flat_file_test.cpp)
target_link_libraries(flat_file_test
    ametsuchi
    libs_common
    )

addtest(block_query_test block_query_test.cpp)
target_link_libraries(block_query_test
    ametsuchi
    ametsuchi_fixture
    libs_common
    shared_model_stateless_validation
    )

addtest(block_query_transfer_test block_query_transfer_test.cpp)
target_link_libraries(block_query_transfer_test
    ametsuchi
    libs_common
    ametsuchi_fixture
    shared_model_stateless_validation
    )

addtest(kv_storage_test kv_storage_test.cpp)
target_link_libraries(kv_storage_test
    ametsuchi
    libs_common
    ametsuchi_fixture
    )

addtest(storage_init_test storage_init_test.cpp)
target_link_libraries(storage_init_test
    ametsuchi
    libs_common
    integration_framework_config_helper
    )

addtest(postgres_options_test postgres_options_test.cpp)
target_link_libraries(postgres_options_test
    ametsuchi
    libs_common
    )

addtest(postgres_executor_test postgres_executor_test.cpp)
target_link_libraries(postgres_executor_test
    integration_framework
    ametsuchi
    libs_common
    )

add_library(ametsuchi_fixture INTERFACE)
target_link_libraries(ametsuchi_fixture INTERFACE
    integration_framework_config_helper
    shared_model_proto_backend
    SOCI::core
    SOCI::postgresql
    )

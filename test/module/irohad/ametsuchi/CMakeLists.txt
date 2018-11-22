addtest(ametsuchi_test ametsuchi_test.cpp)
target_link_libraries(ametsuchi_test
    ametsuchi
    ametsuchi_fixture
    shared_model_stateless_validation
    )

addtest(wsv_query_command_test wsv_query_command_test.cpp)
target_link_libraries(wsv_query_command_test
    ametsuchi
    ametsuchi_fixture
    )

addtest(flat_file_test flat_file_test.cpp)
target_link_libraries(flat_file_test
    ametsuchi
    )

addtest(block_query_test block_query_test.cpp)
target_link_libraries(block_query_test
    ametsuchi
    ametsuchi_fixture
    shared_model_stateless_validation
    )

addtest(block_query_transfer_test block_query_transfer_test.cpp)
target_link_libraries(block_query_transfer_test
    ametsuchi
    ametsuchi_fixture
    shared_model_stateless_validation
    )

addtest(kv_storage_test kv_storage_test.cpp)
target_link_libraries(kv_storage_test
    ametsuchi
    ametsuchi_fixture
    )

addtest(storage_init_test storage_init_test.cpp)
target_link_libraries(storage_init_test
    ametsuchi
    integration_framework_config_helper
    shared_model_proto_backend
    )

addtest(postgres_options_test postgres_options_test.cpp)
target_link_libraries(postgres_options_test
    ametsuchi
    )

addtest(postgres_executor_test postgres_executor_test.cpp)
target_link_libraries(postgres_executor_test
    integration_framework
    ametsuchi
    )

addtest(postgres_query_executor_test postgres_query_executor_test.cpp)
target_link_libraries(postgres_query_executor_test
    ametsuchi_fixture
    ametsuchi
    )

addtest(tx_presence_cache_test tx_presence_cache_test.cpp)
target_link_libraries(tx_presence_cache_test
    ametsuchi
    shared_model_interfaces_factories
    )

add_library(ametsuchi_fixture INTERFACE)
target_link_libraries(ametsuchi_fixture INTERFACE
    integration_framework_config_helper
    shared_model_proto_backend
    SOCI::core
    SOCI::postgresql
    )

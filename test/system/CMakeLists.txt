addtest(irohad_test irohad_test.cpp)
target_link_libraries(irohad_test
        boost
        SOCI::core
        SOCI::postgresql
        rapidjson
        integration_framework_config_helper
        libs_common
        shared_model_proto_backend
        acceptance_fixture
        command_client
        query_client
        keys_manager
        )
add_dependencies(irohad_test irohad)
add_definitions(-DPATHIROHAD="${PROJECT_BINARY_DIR}/bin")
add_definitions(-DPATHEXAMPLE="${PROJECT_SOURCE_DIR}/example")

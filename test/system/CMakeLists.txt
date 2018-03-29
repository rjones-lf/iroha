addtest(irohad_test irohad_test.cpp)
target_link_libraries(irohad_test
        boost
        pqxx
        json_model_converters
        integration_framework_config_helper
        libs_common
        )
add_dependencies(irohad_test irohad)
add_definitions(-DPATHIROHAD="${PROJECT_BINARY_DIR}/bin")

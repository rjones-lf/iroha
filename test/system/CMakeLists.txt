addtest(irohad_test irohad_test.cpp)
target_link_libraries(irohad_test
        boost
        libs_common
        pqxx
        rapidjson
        )
add_dependencies(irohad_test irohad)
add_definitions(-DPATHIROHAD="${PROJECT_BINARY_DIR}/bin")

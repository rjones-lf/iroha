set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/test_bin)

add_subdirectory(consensus)
add_subdirectory(pipeline)

addtest(client_test client_test.cpp)
target_link_libraries(client_test
    pb_model_converters
    json_model_converters
    client
    processors
    server_runner
    )
target_include_directories(client_test PUBLIC
    ${PROJECT_SOURCE_DIR}/iroha-cli
    )

addtest(irohad_test irohad_test.cpp)
target_link_libraries(irohad_test
    boost
    )
add_definitions(-DPATHIROHAD="${PROJECT_BINARY_DIR}/bin")

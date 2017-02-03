
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/test_bin)

SET(CMAKE_BUILD_TYPE Debug)
SET(GTEST_INCLUDE_DIR ${PROJECT_BINARY_DIR}/gtest/src/googletest/googletest/include)

include_directories(
    ${GTEST_INCLUDE_DIR}
)
link_directories(
    ${PROJECT_BINARY_DIR}/lib
)

add_executable(ensure_sumeragi_json_format_test
    ensure_sumeragi_json_format.cpp
)
target_link_libraries(ensure_sumeragi_json_format_test
    iroha_config_with_json
    peer_service_with_json
    ${GTEST_LIBS_DIR}/libgtest.a
    ${GTEST_LIBS_DIR}/libgtest_main.a
    pthread
)

configure_file(inputs/bad_ip.json ${PROJECT_BINARY_DIR}/test/infra/config/inputs/bad_ip.json)
configure_file(inputs/no_group_publicKey.json ${PROJECT_BINARY_DIR}/test/infra/config/inputs/no_group_publicKey.json)
configure_file(inputs/normal_sumeragi.json ${PROJECT_BINARY_DIR}/test/infra/config/inputs/normal_sumeragi.json)
configure_file(inputs/bad_ip2.json ${PROJECT_BINARY_DIR}/test/infra/config/inputs/inputs/bad_ip2.json)
configure_file(inputs/no_me_ip.json ${PROJECT_BINARY_DIR}/test/infra/config/inputs/no_me_ip.json)
configure_file(inputs/useless_group_key.json ${PROJECT_BINARY_DIR}/test/infra/config/inputs/useless_group_key.json)
configure_file(inputs/bad_json.json ${PROJECT_BINARY_DIR}/test/infra/config/inputs/bad_json.json)
configure_file(inputs/no_me_name.json ${PROJECT_BINARY_DIR}/test/infra/config/inputs/no_me_name.json)
configure_file(inputs/useless_me_key.json ${PROJECT_BINARY_DIR}/test/infra/config/inputs/useless_me_key.json)
configure_file(inputs/no_group_ip.json ${PROJECT_BINARY_DIR}/test/infra/config/inputs/no_group_ip.json)
configure_file(inputs/no_me_privateKey.json ${PROJECT_BINARY_DIR}/test/infra/config/inputs/no_me_privateKey.json)
configure_file(inputs/no_group_name.json ${PROJECT_BINARY_DIR}/test/infra/config/inputs/no_group_name.json)
configure_file(inputs/no_me_publicKey.json ${PROJECT_BINARY_DIR}/test/infra/config/inputs/no_me_publicKey.json)

add_test(
    NAME ensure_sumeragi_json_format_test
    COMMAND $<TARGET_FILE:ensure_sumeragi_json_format_test>
)

#
# Copyright Soramitsu Co., Ltd. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0
#

addtest(test_subscriber_testing test_subscriber_testing.cpp)
target_link_libraries(test_subscriber_testing
    rxcpp
    )

add_library(test_logger test_logger.cpp)
target_link_libraries(test_logger
    logger
    logger_manager
    )

add_library(integration_framework
    integration_framework/integration_test_framework.cpp
    integration_framework/iroha_instance.cpp
    integration_framework/fake_peer/fake_peer.cpp
    integration_framework/fake_peer/yac_network_notifier.cpp
    integration_framework/port_guard.cpp
    )
target_link_libraries(integration_framework
    application
    integration_framework_config_helper
    command_client
    common_test_constants
    query_client
    ordering_gate_common
    yac_transport
    shared_model_cryptography_model
    server_runner
    mst_transport
    test_logger
    )

add_library(common_test_constants common_constants.cpp)

target_include_directories(integration_framework PUBLIC ${PROJECT_SOURCE_DIR}/test)

add_library(integration_framework_config_helper
    config_helper.cpp
    )
target_include_directories(integration_framework_config_helper PUBLIC ${PROJECT_SOURCE_DIR}/test)

add_library(framework_sql_query
    sql_query.cpp
    )
target_link_libraries(framework_sql_query
    SOCI::core
    SOCI::postgresql
    logger
    shared_model_proto_backend
    )
target_compile_definitions(framework_sql_query
    PRIVATE SOCI_USE_BOOST HAVE_BOOST
    )

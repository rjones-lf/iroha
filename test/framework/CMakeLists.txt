#
# Copyright Soramitsu Co., Ltd. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0
#

addtest(test_subscriber_testing test_subscriber_testing.cpp)
target_link_libraries(test_subscriber_testing
    rxcpp
    )

add_library(integration_framework
    integration_framework/integration_test_framework.cpp
    integration_framework/iroha_instance.cpp
    integration_framework/fake_peer/fake_peer.cpp
    integration_framework/fake_peer/yac_network_notifier.cpp
    integration_framework/port_guard.cpp
    common_constants.cpp
    )
target_link_libraries(integration_framework
    application
    integration_framework_config_helper
    command_client
    query_client
    ordering_gate_common
    yac_transport
    shared_model_cryptography_model
    server_runner
    mst_transport
    )

target_include_directories(integration_framework PUBLIC ${PROJECT_SOURCE_DIR}/test)

add_library(integration_framework_config_helper config_helper.cpp)
target_include_directories(integration_framework_config_helper PUBLIC ${PROJECT_SOURCE_DIR}/test)

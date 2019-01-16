#
# Copyright Soramitsu Co., Ltd. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0
#

if (IROHA_ROOT_PROJECT)
  addtest(transport_builder_test
      transport_builder_test.cpp
      )
  target_link_libraries(transport_builder_test
      shared_model_proto_builders
      shared_model_stateless_validation
      shared_model_interfaces_factories
      )
endif ()

addtest(proto_peer_builder_test
    common_objects/proto_peer_builder_test.cpp
    )

target_link_libraries(proto_peer_builder_test
    shared_model_proto_builders
    )

addtest(proto_account_builder_test
    common_objects/proto_account_builder_test.cpp
    )

target_link_libraries(proto_account_builder_test
    shared_model_proto_builders
    )

addtest(proto_signature_builder_test
    common_objects/proto_signature_builder_test.cpp
    )

target_link_libraries(proto_signature_builder_test
    shared_model_proto_builders
    )

addtest(proto_asset_builder_test
    common_objects/proto_asset_builder_test.cpp
    )

target_link_libraries(proto_asset_builder_test
    shared_model_proto_builders
    )

addtest(proto_account_asset_builder_test
    common_objects/proto_account_asset_builder_test.cpp
    )

target_link_libraries(proto_account_asset_builder_test
    shared_model_proto_builders
    )

addtest(proto_transaction_responses_builder_test
    transaction_responses/proto_transaction_response_builder.cpp
    )

target_link_libraries(proto_transaction_responses_builder_test
    shared_model_proto_builders
    )

addtest(proto_transaction_responses_factory_test
    transaction_responses/proto_transaction_response_factory_test.cpp
    )

target_link_libraries(proto_transaction_responses_factory_test
    shared_model_proto_backend
    )

addtest(block_builder_test
    block_builder_test.cpp
    )

target_link_libraries(block_builder_test
    shared_model_proto_builders
    shared_model_stateless_validation
    )

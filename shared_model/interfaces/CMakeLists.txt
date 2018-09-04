#
# Copyright Soramitsu Co., Ltd. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0
#

add_library(shared_model_interfaces
    impl/permissions.cpp
    commands/impl/add_asset_quantity.cpp
    commands/impl/add_peer.cpp
    commands/impl/add_signatory.cpp
    commands/impl/append_role.cpp
    commands/impl/command.cpp
    commands/impl/create_account.cpp
    commands/impl/create_asset.cpp
    commands/impl/create_domain.cpp
    commands/impl/create_role.cpp
    commands/impl/detach_role.cpp
    commands/impl/grant_permission.cpp
    commands/impl/remove_signatory.cpp
    commands/impl/revoke_permission.cpp
    commands/impl/set_account_detail.cpp
    commands/impl/set_quorum.cpp
    commands/impl/subtract_asset_quantity.cpp
    commands/impl/transfer_asset.cpp
    queries/impl/query.cpp
    queries/impl/get_account.cpp
    queries/impl/get_account_asset_transactions.cpp
    queries/impl/get_account_assets.cpp
    queries/impl/get_account_detail.cpp
    queries/impl/get_account_transactions.cpp
    queries/impl/get_asset_info.cpp
    queries/impl/get_role_permissions.cpp
    queries/impl/get_roles.cpp
    queries/impl/get_signatories.cpp
    queries/impl/get_transactions.cpp
    queries/impl/get_pending_transactions.cpp
    queries/impl/blocks_query.cpp
    queries/impl/query_payload_meta.cpp
    common_objects/impl/amount.cpp
    )

if (IROHA_ROOT_PROJECT)
  target_sources(shared_model_interfaces PRIVATE
      query_responses/impl/account_asset_response.cpp
      query_responses/impl/account_detail_response.cpp
      query_responses/impl/account_response.cpp
      query_responses/impl/asset_response.cpp
      query_responses/impl/error_query_response.cpp
      query_responses/impl/query_response.cpp
      query_responses/impl/role_permissions.cpp
      query_responses/impl/roles_response.cpp
      query_responses/impl/signatories_response.cpp
      query_responses/impl/transactions_response.cpp
      query_responses/impl/block_error_response.cpp
      query_responses/impl/block_query_response.cpp
      query_responses/impl/block_response.cpp
      iroha_internal/block_variant.cpp
      iroha_internal/transaction_sequence.cpp
      iroha_internal/transaction_batch.cpp
      iroha_internal/transaction_batch_factory.cpp
      )
endif ()

target_link_libraries(shared_model_interfaces
    shared_model_cryptography
    schema
    ${Boost_LIBRARIES}
    )

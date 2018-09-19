/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "backend/protobuf/proto_query_response_factory.hpp"
#include "backend/protobuf/permissions.hpp"
#include "backend/protobuf/query_responses/proto_block_query_response.hpp"
#include "backend/protobuf/query_responses/proto_query_response.hpp"

std::unique_ptr<shared_model::interface::QueryResponse>
shared_model::proto::ProtoQueryResponseFactory::createAccountAssetResponse(
    std::vector<std::shared_ptr<shared_model::interface::AccountAsset>> assets,
    const crypto::Hash &query_hash) {
  iroha::protocol::QueryResponse protocol_query_response;
  protocol_query_response.set_query_hash(query_hash.hex());

  iroha::protocol::AccountAssetResponse *protocol_specific_response =
      protocol_query_response.mutable_account_assets_response();
  for (const auto &asset : assets) {
    protocol_specific_response->add_account_assets()->CopyFrom(
        std::static_pointer_cast<shared_model::proto::AccountAsset>(asset)
            ->getTransport());
  }

  return std::make_unique<shared_model::proto::QueryResponse>(
      std::move(protocol_query_response));
}

std::unique_ptr<shared_model::interface::QueryResponse>
shared_model::proto::ProtoQueryResponseFactory::createAccountDetailResponse(
    shared_model::interface::types::DetailType account_detail,
    const crypto::Hash &query_hash) {
  iroha::protocol::QueryResponse protocol_query_response;
  protocol_query_response.set_query_hash(query_hash.hex());

  iroha::protocol::AccountDetailResponse *protocol_specific_response =
      protocol_query_response.mutable_account_detail_response();
  protocol_specific_response->set_detail(account_detail);

  return std::make_unique<shared_model::proto::QueryResponse>(
      std::move(protocol_query_response));
}

std::unique_ptr<shared_model::interface::QueryResponse>
shared_model::proto::ProtoQueryResponseFactory::createAccountResponse(
    std::unique_ptr<shared_model::interface::Account> account,
    std::vector<std::string> roles,
    const crypto::Hash &query_hash) {
  iroha::protocol::QueryResponse protocol_query_response;
  protocol_query_response.set_query_hash(query_hash.hex());

  iroha::protocol::AccountResponse *protocol_specific_response =
      protocol_query_response.mutable_account_response();
  *protocol_specific_response->mutable_account() =
      static_cast<shared_model::proto::Account *>(account.release())
          ->getTransport();
  for (const auto &role : roles) {
    protocol_specific_response->add_account_roles(role);
  }

  return std::make_unique<shared_model::proto::QueryResponse>(
      std::move(protocol_query_response));
}

std::unique_ptr<shared_model::interface::QueryResponse>
shared_model::proto::ProtoQueryResponseFactory::createErrorQueryResponse(
    ErrorQueryType error_type,
    std::string error_msg,
    const crypto::Hash &query_hash) {
  iroha::protocol::QueryResponse protocol_query_response;
  protocol_query_response.set_query_hash(query_hash.hex());

  iroha::protocol::ErrorResponse_Reason reason;
  switch (error_type) {
    case ErrorQueryType::kStatelessFailed:
      reason = iroha::protocol::ErrorResponse_Reason_STATELESS_INVALID;
      break;
    case ErrorQueryType::kStatefulFailed:
      reason = iroha::protocol::ErrorResponse_Reason_STATEFUL_INVALID;
      break;
    case ErrorQueryType::kNoAccount:
      reason = iroha::protocol::ErrorResponse_Reason_NO_ACCOUNT;
      break;
    case ErrorQueryType::kNoAccountAssets:
      reason = iroha::protocol::ErrorResponse_Reason_NO_ACCOUNT_ASSETS;
      break;
    case ErrorQueryType::kNoAccountDetail:
      reason = iroha::protocol::ErrorResponse_Reason_NO_ACCOUNT_DETAIL;
      break;
    case ErrorQueryType::kNoSignatories:
      reason = iroha::protocol::ErrorResponse_Reason_NO_SIGNATORIES;
      break;
    case ErrorQueryType::kNotSupported:
      reason = iroha::protocol::ErrorResponse_Reason_NOT_SUPPORTED;
      break;
    case ErrorQueryType::kNoAsset:
      reason = iroha::protocol::ErrorResponse_Reason_NO_ASSET;
      break;
    case ErrorQueryType::kNoRoles:
      reason = iroha::protocol::ErrorResponse_Reason_NO_ROLES;
      break;
  }
  iroha::protocol::ErrorResponse *protocol_specific_response =
      protocol_query_response.mutable_error_response();
  protocol_specific_response->set_reason(reason);
  protocol_specific_response->set_message(std::move(error_msg));

  return std::make_unique<shared_model::proto::QueryResponse>(
      std::move(protocol_query_response));
}

std::unique_ptr<shared_model::interface::QueryResponse>
shared_model::proto::ProtoQueryResponseFactory::createSignatoriesResponse(
    std::vector<shared_model::interface::types::PubkeyType> signatories,
    const crypto::Hash &query_hash) {
  iroha::protocol::QueryResponse protocol_query_response;
  protocol_query_response.set_query_hash(query_hash.hex());

  iroha::protocol::SignatoriesResponse *protocol_specific_response =
      protocol_query_response.mutable_signatories_response();
  for (const auto &key : signatories) {
    const auto &blob = key.blob();
    protocol_specific_response->add_keys(blob.data(), blob.size());
  }

  return std::make_unique<shared_model::proto::QueryResponse>(
      std::move(protocol_query_response));
}

std::unique_ptr<shared_model::interface::QueryResponse>
shared_model::proto::ProtoQueryResponseFactory::createTransactionsResponse(
    std::vector<std::shared_ptr<shared_model::interface::Transaction>>
        transactions,
    const crypto::Hash &query_hash) {
  iroha::protocol::QueryResponse protocol_query_response;
  protocol_query_response.set_query_hash(query_hash.hex());

  iroha::protocol::TransactionsResponse *protocol_specific_response =
      protocol_query_response.mutable_transactions_response();
  for (const auto &tx : transactions) {
    protocol_specific_response->add_transactions()->CopyFrom(
        std::static_pointer_cast<shared_model::proto::Transaction>(tx)
            ->getTransport());
  }

  return std::make_unique<shared_model::proto::QueryResponse>(
      std::move(protocol_query_response));
}

std::unique_ptr<shared_model::interface::QueryResponse>
shared_model::proto::ProtoQueryResponseFactory::createAssetResponse(
    std::unique_ptr<shared_model::interface::Asset> asset,
    const crypto::Hash &query_hash) {
  iroha::protocol::QueryResponse protocol_query_response;
  protocol_query_response.set_query_hash(query_hash.hex());

  iroha::protocol::AssetResponse *protocol_specific_response =
      protocol_query_response.mutable_asset_response();
  *protocol_specific_response->mutable_asset() =
      static_cast<shared_model::proto::Asset *>(asset.release())
          ->getTransport();

  return std::make_unique<shared_model::proto::QueryResponse>(
      std::move(protocol_query_response));
}

std::unique_ptr<shared_model::interface::QueryResponse>
shared_model::proto::ProtoQueryResponseFactory::createRolesResponse(
    std::vector<shared_model::interface::types::RoleIdType> roles,
    const crypto::Hash &query_hash) {
  iroha::protocol::QueryResponse protocol_query_response;
  protocol_query_response.set_query_hash(query_hash.hex());

  iroha::protocol::RolesResponse *protocol_specific_response =
      protocol_query_response.mutable_roles_response();
  for (const auto &role : roles) {
    protocol_specific_response->add_roles(role);
  }

  return std::make_unique<shared_model::proto::QueryResponse>(
      std::move(protocol_query_response));
}

std::unique_ptr<shared_model::interface::QueryResponse>
shared_model::proto::ProtoQueryResponseFactory::createRolePermissionsResponse(
    shared_model::interface::RolePermissionSet role_permissions,
    const crypto::Hash &query_hash) {
  iroha::protocol::QueryResponse protocol_query_response;
  protocol_query_response.set_query_hash(query_hash.hex());

  iroha::protocol::RolePermissionsResponse *protocol_specific_response =
      protocol_query_response.mutable_role_permissions_response();
  for (size_t i = 0; i < role_permissions.size(); ++i) {
    auto perm = static_cast<interface::permissions::Role>(i);
    if (role_permissions.test(perm)) {
      protocol_specific_response->add_permissions(
          shared_model::proto::permissions::toTransport(perm));
    }
  }

  return std::make_unique<shared_model::proto::QueryResponse>(
      std::move(protocol_query_response));
}

std::unique_ptr<shared_model::interface::BlockQueryResponse>
shared_model::proto::ProtoQueryResponseFactory::createBlockQueryResponse(
    std::unique_ptr<shared_model::interface::Block> block) {
  iroha::protocol::BlockQueryResponse protocol_query_response;

  iroha::protocol::BlockResponse *protocol_specific_response =
      protocol_query_response.mutable_block_response();
  *protocol_specific_response->mutable_block() =
      static_cast<shared_model::proto::Block *>(block.release())
          ->getTransport();

  return std::make_unique<shared_model::proto::BlockQueryResponse>(
      std::move(protocol_query_response));
}

std::unique_ptr<shared_model::interface::BlockQueryResponse>
shared_model::proto::ProtoQueryResponseFactory::createBlockQueryResponse(
    std::string error_message) {
  iroha::protocol::BlockQueryResponse protocol_query_response;

  iroha::protocol::BlockErrorResponse *protocol_specific_response =
      protocol_query_response.mutable_block_error_response();
  protocol_specific_response->set_message(error_message);

  return std::make_unique<shared_model::proto::BlockQueryResponse>(
      std::move(protocol_query_response));
}

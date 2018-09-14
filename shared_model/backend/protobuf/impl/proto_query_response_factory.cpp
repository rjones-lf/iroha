/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "backend/protobuf/proto_query_response_factory.hpp"
#include "backend/protobuf/permissions.hpp"

std::unique_ptr<shared_model::interface::AccountAssetResponse>
shared_model::proto::ProtoQueryResponseFactory::createAccountAssetResponse(
    const std::vector<shared_model::interface::AccountAsset> &assets) {
  iroha::protocol::QueryResponse protocol_response;

  iroha::protocol::AccountAssetResponse *query_response =
      protocol_response.mutable_account_assets_response();
  for (auto &asset : assets) {
    query_response->add_account_assets()->CopyFrom(
        static_cast<shared_model::proto::AccountAsset>(asset).getTransport());
  }

  return std::make_unique<shared_model::proto::AccountAssetResponse>(
      query_response);
}

std::unique_ptr<shared_model::interface::AccountDetailResponse>
shared_model::proto::ProtoQueryResponseFactory::createAccountDetailResponse(
    const shared_model::interface::types::DetailType &account_detail) {
  iroha::protocol::QueryResponse protocol_response;

  iroha::protocol::AccountDetailResponse *query_response =
      protocol_response.mutable_account_detail_response();
  query_response->set_detail(account_detail);

  return std::make_unique<shared_model::proto::AccountDetailResponse>(
      query_response);
}

std::unique_ptr<shared_model::interface::AccountResponse>
shared_model::proto::ProtoQueryResponseFactory::createAccountResponse(
    const shared_model::interface::Account &account,
    const std::vector<std::string> &roles) {
  iroha::protocol::QueryResponse protocol_response;

  iroha::protocol::AccountResponse *query_response =
      protocol_response.mutable_account_response();
  query_response->mutable_account()->CopyFrom(
      static_cast<shared_model::proto::Account>(account).getTransport());
  for (const auto &role : roles) {
    query_response->add_account_roles(role);
  }

  return std::make_unique<shared_model::proto::AccountResponse>(query_response);
}

std::unique_ptr<shared_model::interface::ErrorQueryResponse>
shared_model::proto::ProtoQueryResponseFactory::createErrorQueryResponse() {
  iroha::protocol::QueryResponse protocol_response;

  constexpr iroha::protocol::ErrorResponse_Reason reason =
      iroha::protocol::ErrorResponse_Reason_STATELESS_INVALID;
  iroha::protocol::ErrorResponse *query_response =
      protocol_response.mutable_error_response();
  query_response->set_reason(reason);

  return std::make_unique<shared_model::proto::ErrorQueryResponse>(
      query_response);
}

std::unique_ptr<shared_model::interface::SignatoriesResponse>
shared_model::proto::ProtoQueryResponseFactory::createSignatoriesResponse(
    const std::vector<shared_model::interface::types::PubkeyType>
        &signatories) {
  iroha::protocol::QueryResponse protocol_response;

  iroha::protocol::SignatoriesResponse *query_response =
      protocol_response.mutable_signatories_response();
  for (const auto &key : signatories) {
    const auto &blob = key.blob();
    query_response->add_keys(blob.data(), blob.size());
  }

  return std::make_unique<shared_model::proto::SignatoriesResponse>(
      query_response);
}

std::unique_ptr<shared_model::interface::TransactionsResponse>
shared_model::proto::ProtoQueryResponseFactory::createTransactionsResponse(
    const std::vector<shared_model::interface::Transaction> &transactions) {
  iroha::protocol::QueryResponse protocol_response;

  iroha::protocol::TransactionsResponse *query_response =
      protocol_response.mutable_transactions_response();
  for (const auto &tx : transactions) {
    query_response->add_transactions()->CopyFrom(
        static_cast<shared_model::proto::Transaction>(tx).getTransport());
  }

  return std::make_unique<shared_model::proto::TransactionsResponse>(
      query_response);
}

std::unique_ptr<shared_model::interface::AssetResponse>
shared_model::proto::ProtoQueryResponseFactory::createAssetResponse(
    const std::string &asset_id,
    const std::string &domain_id,
    const uint32_t precision) {
  iroha::protocol::QueryResponse protocol_response;

  iroha::protocol::AssetResponse *query_response =
      protocol_response.mutable_asset_response();
  auto asset = query_response->mutable_asset();
  asset->set_asset_id(asset_id);
  asset->set_domain_id(domain_id);
  asset->set_precision(precision);

  return std::make_unique<shared_model::proto::AssetResponse>(query_response);
}

std::unique_ptr<shared_model::interface::RolesResponse>
shared_model::proto::ProtoQueryResponseFactory::createRolesResponse(
    const std::vector<shared_model::interface::types::RoleIdType> &roles) {
  iroha::protocol::QueryResponse protocol_response;

  iroha::protocol::RolesResponse *query_response =
      protocol_response.mutable_roles_response();
  for (const auto &role : roles) {
    query_response->add_roles(role);
  }

  return std::make_unique<shared_model::proto::RolesResponse>(query_response);
}

std::unique_ptr<shared_model::interface::RolePermissionsResponse>
shared_model::proto::ProtoQueryResponseFactory::createRolePermissionsResponse(
    const shared_model::interface::RolePermissionSet &role_permissions) {
  iroha::protocol::QueryResponse protocol_response;

  iroha::protocol::RolePermissionsResponse *query_response =
      protocol_response.mutable_role_permissions_response();
  for (size_t i = 0; i < role_permissions.size(); ++i) {
    auto perm = static_cast<interface::permissions::Role>(i);
    if (role_permissions.test(perm)) {
      query_response->add_permissions(
          shared_model::proto::permissions::toTransport(perm));
    }
  }

  return std::make_unique<shared_model::proto::RolePermissionsResponse>(
      query_response);
}

std::unique_ptr<shared_model::interface::BlockQueryResponse>
shared_model::proto::ProtoQueryResponseFactory::createBlockQueryResponse(
    const shared_model::interface::Block &block) {
  const auto &proto_block =
      static_cast<const shared_model::proto::Block &>(block);
  iroha::protocol::BlockResponse protocol_response;

  protocol_response.set_allocated_block(
      new iroha::protocol::Block(proto_block.getTransport()));

  return std::make_unique<shared_model::proto::BlockQueryResponse>(
      std::move(protocol_response));
}

std::unique_ptr<shared_model::interface::BlockQueryResponse>
shared_model::proto::ProtoQueryResponseFactory::createBlockQueryResponse(
    const std::string &error_message) {
  iroha::protocol::BlockErrorResponse protocol_response;

  protocol_response.set_message(error_message);

  return std::make_unique<shared_model::proto::BlockQueryResponse>(
      std::move(protocol_response));
}

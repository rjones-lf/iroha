/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "backend/protobuf/proto_query_response_factory.hpp"

std::unique_ptr<shared_model::interface::AccountAssetResponse>
shared_model::proto::ProtoQueryResponseFactory::createAccountAssetResponse() {}

std::unique_ptr<shared_model::interface::AccountDetailResponse>
shared_model::proto::ProtoQueryResponseFactory::createAccountDetailResponse() {}

std::unique_ptr<shared_model::interface::AccountResponse>
shared_model::proto::ProtoQueryResponseFactory::createAccountAccountResponse() {
}

std::unique_ptr<shared_model::interface::ErrorQueryResponse>
shared_model::proto::ProtoQueryResponseFactory::createErrorQueryResponse() {}

std::unique_ptr<shared_model::interface::SignatoriesResponse>
shared_model::proto::ProtoQueryResponseFactory::createSignatoriesResponse() {}

std::unique_ptr<shared_model::interface::TransactionsResponse>
shared_model::proto::ProtoQueryResponseFactory::createTransactionsResponse() {}

std::unique_ptr<shared_model::interface::AssetResponse>
shared_model::proto::ProtoQueryResponseFactory::createAssetResponse() {}

std::unique_ptr<shared_model::interface::RolesResponse>
shared_model::proto::ProtoQueryResponseFactory::createRolesResponse() {}

std::unique_ptr<shared_model::interface::RolePermissionsResponse> shared_model::
    proto::ProtoQueryResponseFactory::createRolePermissionsResponse() {}

std::unique_ptr<shared_model::interface::BlockQueryResponse>
shared_model::proto::ProtoQueryResponseFactory::createBlockQueryResponse(
    const shared_model::interface::Block &block) {
  const auto &proto_block =
      static_cast<const shared_model::proto::Block &>(block);
  iroha::protocol::BlockResponse response;
  response.set_allocated_block(
      new iroha::protocol::Block(proto_block.getTransport()));
  return std::make_unique<shared_model::proto::BlockQueryResponse>(
      std::move(response));
}

std::unique_ptr<shared_model::interface::BlockQueryResponse>
shared_model::proto::ProtoQueryResponseFactory::createBlockQueryResponse(
    const std::string &error_message) {
  iroha::protocol::BlockErrorResponse response;
  response.set_message(error_message);
  return std::make_unique<shared_model::proto::BlockQueryResponse>(
      std::move(response));
}

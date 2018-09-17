/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_PROTO_QUERY_RESPONSE_FACTORY_HPP
#define IROHA_PROTO_QUERY_RESPONSE_FACTORY_HPP

#include "backend/protobuf/query_responses/proto_block_query_response.hpp"
#include "backend/protobuf/query_responses/proto_query_response.hpp"
#include "interfaces/iroha_internal/query_response_factory.hpp"

namespace shared_model {
  namespace proto {

    class ProtoQueryResponseFactory : public interface::QueryResponseFactory {
     public:
      std::unique_ptr<interface::AccountAssetResponse>
      createAccountAssetResponse(
          const std::vector<
              std::shared_ptr<shared_model::interface::AccountAsset>> &assets)
          override;

      std::unique_ptr<interface::AccountDetailResponse>
      createAccountDetailResponse(
          const interface::types::DetailType &account_detail) override;

      std::unique_ptr<interface::AccountResponse> createAccountResponse(
          const interface::Account &account,
          const std::vector<std::string> &roles) override;

      std::unique_ptr<interface::ErrorQueryResponse> createErrorQueryResponse(
          ErrorQueryType error_type) override;

      std::unique_ptr<interface::SignatoriesResponse> createSignatoriesResponse(
          const std::vector<interface::types::PubkeyType> &signatories)
          override;

      std::unique_ptr<interface::TransactionsResponse>
      createTransactionsResponse(
          const std::vector<
              std::shared_ptr<shared_model::interface::Transaction>>
              &transactions) override;

      std::unique_ptr<interface::AssetResponse> createAssetResponse(
          const shared_model::interface::Asset &asset) override;

      std::unique_ptr<interface::RolesResponse> createRolesResponse(
          const std::vector<interface::types::RoleIdType> &roles) override;

      std::unique_ptr<interface::RolePermissionsResponse>
      createRolePermissionsResponse(
          const interface::RolePermissionSet &role_permissions) override;

      std::unique_ptr<interface::BlockQueryResponse> createBlockQueryResponse(
          const interface::Block &block) override;

      std::unique_ptr<interface::BlockQueryResponse> createBlockQueryResponse(
          const std::string &error_message) override;
    };

  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_PROTO_QUERY_RESPONSE_FACTORY_HPP

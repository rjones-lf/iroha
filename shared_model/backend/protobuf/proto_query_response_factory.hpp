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
      createAccountAssetResponse() override;

      std::unique_ptr<interface::AccountDetailResponse>
      createAccountDetailResponse() override;

      std::unique_ptr<interface::AccountResponse> createAccountAccountResponse()
          override;

      std::unique_ptr<interface::ErrorQueryResponse> createErrorQueryResponse()
          override;

      std::unique_ptr<interface::SignatoriesResponse>
      createSignatoriesResponse() override;

      std::unique_ptr<interface::TransactionsResponse>
      createTransactionsResponse() override;

      std::unique_ptr<interface::AssetResponse> createAssetResponse() override;

      std::unique_ptr<interface::RolesResponse> createRolesResponse() override;

      std::unique_ptr<interface::RolePermissionsResponse>
      createRolePermissionsResponse() override;

      std::unique_ptr<interface::BlockQueryResponse> createBlockQueryResponse(
          const interface::Block &block) override;

      std::unique_ptr<interface::BlockQueryResponse> createBlockQueryResponse(
          const std::string &error_message) override;
    };

  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_PROTO_QUERY_RESPONSE_FACTORY_HPP

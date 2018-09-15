/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_QUERY_RESPONSE_FACTORY_H
#define IROHA_QUERY_RESPONSE_FACTORY_H

#include <memory>
#include "interfaces/query_responses/block_query_response.hpp"
#include "interfaces/query_responses/query_response.hpp"

namespace shared_model {
  namespace interface {

    /**
     * Factory for building query responses
     */
    class QueryResponseFactory {
     public:
      virtual ~QueryResponseFactory() = default;

      virtual std::unique_ptr<AccountAssetResponse> createAccountAssetResponse(
          const std::vector<
              std::shared_ptr<shared_model::interface::AccountAsset>>
              &assets) = 0;

      virtual std::unique_ptr<AccountDetailResponse>
      createAccountDetailResponse(const types::DetailType &account_detail) = 0;

      virtual std::unique_ptr<AccountResponse> createAccountResponse(
          const Account &account, const std::vector<std::string> &roles) = 0;

      virtual std::unique_ptr<ErrorQueryResponse>
      createErrorQueryResponse() = 0;

      virtual std::unique_ptr<SignatoriesResponse> createSignatoriesResponse(
          const std::vector<types::PubkeyType> &signatories) = 0;

      virtual std::unique_ptr<TransactionsResponse> createTransactionsResponse(
          const std::vector<Transaction> &transactions) = 0;

      virtual std::unique_ptr<AssetResponse> createAssetResponse(
          const std::string &asset_id,
          const std::string &domain_id,
          const uint32_t precision) = 0;

      virtual std::unique_ptr<RolesResponse> createRolesResponse(
          const std::vector<types::RoleIdType> &roles) = 0;

      virtual std::unique_ptr<RolePermissionsResponse>
      createRolePermissionsResponse(
          const RolePermissionSet &role_permissions) = 0;

      /**
       * Create a block query response
       * @param block to be placed inside the response
       * @return pointer to the response
       */
      virtual std::unique_ptr<BlockQueryResponse> createBlockQueryResponse(
          const interface::Block &block) = 0;

      /**
       * Create a block query response
       * @param error_message to be placed inside the response
       * @return pointer to the response
       */
      virtual std::unique_ptr<BlockQueryResponse> createBlockQueryResponse(
          const std::string &error_message) = 0;
    };

  }  // namespace interface
}  // namespace shared_model

#endif  // IROHA_QUERY_RESPONSE_FACTORY_H

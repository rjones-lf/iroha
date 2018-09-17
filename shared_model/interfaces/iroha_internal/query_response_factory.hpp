/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_QUERY_RESPONSE_FACTORY_HPP
#define IROHA_QUERY_RESPONSE_FACTORY_HPP

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

      /**
       * Create response for account asset query
       * @param assets to be inserted into the response
       * @return account asset response
       */
      virtual std::unique_ptr<AccountAssetResponse> createAccountAssetResponse(
          const std::vector<
              std::shared_ptr<shared_model::interface::AccountAsset>>
              &assets) = 0;

      /**
       * Create response for account detail query
       * @param account_detail to be inserted into the response
       * @return account detail response
       */
      virtual std::unique_ptr<AccountDetailResponse>
      createAccountDetailResponse(const types::DetailType &account_detail) = 0;

      /**
       * Create response for account query
       * @param account to be inserted into the response
       * @param roles to be inserted into the response
       * @return account response
       */
      virtual std::unique_ptr<AccountResponse> createAccountResponse(
          const Account &account, const std::vector<std::string> &roles) = 0;

      /**
       * Create response for failed query
       * @return error response
       */
      virtual std::unique_ptr<ErrorQueryResponse>
      createErrorQueryResponse() = 0;

      /**
       * Create response for signatories query
       * @param signatories to be inserted into the response
       * @return signatories response
       */
      virtual std::unique_ptr<SignatoriesResponse> createSignatoriesResponse(
          const std::vector<types::PubkeyType> &signatories) = 0;

      /**
       * Create response for transactions query
       * @param transactions to be inserted into the response
       * @return transactions response
       */
      virtual std::unique_ptr<TransactionsResponse> createTransactionsResponse(
          const std::vector<
              std::shared_ptr<shared_model::interface::Transaction>>
              &transactions) = 0;

      /**
       * Create response for asset query
       * @param asset to be inserted into the response
       * @return asset response
       */
      virtual std::unique_ptr<AssetResponse> createAssetResponse(
          const Asset &asset) = 0;

      /**
       * Create response for roles query
       * @param roles to be inserted into the response
       * @return roles response
       */
      virtual std::unique_ptr<RolesResponse> createRolesResponse(
          const std::vector<types::RoleIdType> &roles) = 0;

      /**
       * Create response for role permissions query
       * @param role_permissions to be inserted into the response
       * @return role permissions response
       */
      virtual std::unique_ptr<RolePermissionsResponse>
      createRolePermissionsResponse(
          const RolePermissionSet &role_permissions) = 0;

      /**
       * Create response for block query with block
       * @param block to be inserted into the response
       * @return block query response with block
       */
      virtual std::unique_ptr<BlockQueryResponse> createBlockQueryResponse(
          const interface::Block &block) = 0;

      /**
       * Create response for block query with error
       * @param error_message to be inserted into the response
       * @return block query response with error
       */
      virtual std::unique_ptr<BlockQueryResponse> createBlockQueryResponse(
          const std::string &error_message) = 0;
    };

  }  // namespace interface
}  // namespace shared_model

#endif  // IROHA_QUERY_RESPONSE_FACTORY_HPP

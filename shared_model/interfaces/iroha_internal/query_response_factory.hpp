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
  namespace crypto {
    class Hash;
  }
  namespace interface {
    class Block;
  }
}  // namespace shared_model

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
       * @param query_hash - hash of the query, for which response is created
       * @return account asset response
       */
      virtual std::unique_ptr<QueryResponse> createAccountAssetResponse(
          std::vector<std::unique_ptr<shared_model::interface::AccountAsset>>
              assets,
          const crypto::Hash &query_hash) = 0;

      /**
       * Create response for account detail query
       * @param account_detail to be inserted into the response
       * @param query_hash - hash of the query, for which response is created
       * @return account detail response
       */
      virtual std::unique_ptr<QueryResponse> createAccountDetailResponse(
          types::DetailType account_detail, const crypto::Hash &query_hash) = 0;

      /**
       * Create response for account query
       * @param account to be inserted into the response
       * @param roles to be inserted into the response
       * @param query_hash - hash of the query, for which response is created
       * @return account response
       */
      virtual std::unique_ptr<QueryResponse> createAccountResponse(
          std::unique_ptr<Account> account,
          std::vector<std::string> roles,
          const crypto::Hash &query_hash) = 0;

      /**
       * Describes type of error to be placed inside the error query response
       */
      enum class ErrorQueryType {
        kStatelessFailed,
        kStatefulFailed,
        kNoAccount,
        kNoAccountAssets,
        kNoAccountDetail,
        kNoSignatories,
        kNotSupported,
        kNoAsset,
        kNoRoles
      };
      /**
       * Create response for failed query
       * @param error_type - type of error to be inserted into the response
       * @param error_msg - message, which is to be set in the response
       * @param query_hash - hash of the query, for which response is created
       * @return error response
       */
      virtual std::unique_ptr<QueryResponse> createErrorQueryResponse(
          ErrorQueryType error_type,
          std::string error_msg,
          const crypto::Hash &query_hash) = 0;

      /**
       * Create response for signatories query
       * @param signatories to be inserted into the response
       * @param query_hash - hash of the query, for which response is created
       * @return signatories response
       */
      virtual std::unique_ptr<QueryResponse> createSignatoriesResponse(
          std::vector<types::PubkeyType> signatories,
          const crypto::Hash &query_hash) = 0;

      /**
       * Create response for transactions query
       * @param transactions to be inserted into the response
       * @param query_hash - hash of the query, for which response is created
       * @return transactions response
       */
      virtual std::unique_ptr<QueryResponse> createTransactionsResponse(
          std::vector<std::unique_ptr<shared_model::interface::Transaction>>
              transactions,
          const crypto::Hash &query_hash) = 0;

      /**
       * Create response for asset query
       * @param asset to be inserted into the response
       * @param query_hash - hash of the query, for which response is created
       * @return asset response
       */
      virtual std::unique_ptr<QueryResponse> createAssetResponse(
          std::unique_ptr<Asset> asset, const crypto::Hash &query_hash) = 0;

      /**
       * Create response for roles query
       * @param roles to be inserted into the response
       * @param query_hash - hash of the query, for which response is created
       * @return roles response
       */
      virtual std::unique_ptr<QueryResponse> createRolesResponse(
          std::vector<types::RoleIdType> roles,
          const crypto::Hash &query_hash) = 0;

      /**
       * Create response for role permissions query
       * @param role_permissions to be inserted into the response
       * @param query_hash - hash of the query, for which response is created
       * @return role permissions response
       */
      virtual std::unique_ptr<QueryResponse> createRolePermissionsResponse(
          RolePermissionSet role_permissions,
          const crypto::Hash &query_hash) = 0;

      /**
       * Create response for block query with block
       * @param block to be inserted into the response
       * @return block query response with block
       */
      virtual std::unique_ptr<BlockQueryResponse> createBlockQueryResponse(
          std::unique_ptr<Block> block) = 0;

      /**
       * Create response for block query with error
       * @param error_message to be inserted into the response
       * @return block query response with error
       */
      virtual std::unique_ptr<BlockQueryResponse> createBlockQueryResponse(
          std::string error_message) = 0;
    };

  }  // namespace interface
}  // namespace shared_model

#endif  // IROHA_QUERY_RESPONSE_FACTORY_HPP

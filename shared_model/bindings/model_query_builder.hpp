/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_SHARED_MODEL_MODEL_QUERY_BUILDER_HPP
#define IROHA_SHARED_MODEL_MODEL_QUERY_BUILDER_HPP

#include <boost/optional.hpp>
#include "builders/protobuf/queries.hpp"
#include "builders/protobuf/unsigned_proto.hpp"

namespace shared_model {
  namespace bindings {
    /**
     * Wrapper class for query builder. Designed only for SWIG bindings,
     * don't use in other cases.
     */
    class ModelQueryBuilder {
     private:
      template <int Sp>
      explicit ModelQueryBuilder(const proto::TemplateQueryBuilder<Sp> &o)
          : builder_(o) {}

     public:
      ModelQueryBuilder();

      /**
       * Sets time of query creation (Unix time in milliseconds)
       * @param created_time - time to set
       * @return builder with created time field appended
       */
      ModelQueryBuilder createdTime(
          interface::types::TimestampType created_time);

      /**
       * Sets account id of query creator
       * @param creator_account_id - account of query creator
       * @return builder with query account creator field appended
       */
      ModelQueryBuilder creatorAccountId(
          const interface::types::AccountIdType &creator_account_id);

      /**
       * Sets query counter
       * @param query_counter - number to set as a query counter
       * @return builder with query counter field appended
       */
      ModelQueryBuilder queryCounter(
          interface::types::CounterType query_counter);

      /**
       * Queries state of account
       * @param account_id - id of account to query
       * @return builder with getAccount query inside
       */
      ModelQueryBuilder getAccount(
          const interface::types::AccountIdType &account_id);

      /**
       * Queries signatories of account
       * @param account_id - id of account to query
       * @return builder with getSignatories query inside
       */
      ModelQueryBuilder getSignatories(
          const interface::types::AccountIdType &account_id);

      /**
       * Queries account transaction collection
       * @param account_id - id of account to query
       * @return builder with getAccountTransactions query inside
       */
      ModelQueryBuilder getAccountTransactions(
          const interface::types::AccountIdType &account_id,
          interface::types::TransactionsNumberType page_size,
          const boost::optional<interface::types::HashType> &first_hash =
              boost::none);

      /**
       * Queries account transaction collection for a given asset
       * @param account_id - id of account to query
       * @param asset_id - asset id to query about
       * @return builder with getAccountAssetTransactions query inside
       */
      ModelQueryBuilder getAccountAssetTransactions(
          const interface::types::AccountIdType &account_id,
          const interface::types::AssetIdType &asset_id,
          interface::types::TransactionsNumberType page_size,
          const boost::optional<interface::types::HashType> &first_hash =
              boost::none);

      /**
       * Queries balance of specific asset for given account
       * @param account_id - id of account to query
       * @return builder with getAccountAssets query inside
       */
      ModelQueryBuilder getAccountAssets(
          const interface::types::AccountIdType &account_id);

      /**
       * Queries available roles in the system
       * @return builder with getRoles query inside
       */
      ModelQueryBuilder getRoles();

      /**
       * Queries info about given asset
       * @param asset_id - asset id to query about
       * @return builder with getAssetInfo query inside
       */
      ModelQueryBuilder getAssetInfo(
          const interface::types::AssetIdType &asset_id);

      /**
       * Queries list of permissions for given role
       * @param role_id - role id to query about
       * @return builder with getRolePermissions query inside
       */
      ModelQueryBuilder getRolePermissions(
          const interface::types::RoleIdType &role_id);

      /**
       * Queries transactions for given hashes
       * @param hashes - list of transaction hashes to query
       * @return builder with getTransactions query inside
       */
      ModelQueryBuilder getTransactions(
          const std::vector<crypto::Hash> &hashes);

      /**
       * Retrieves details for a given account
       * @param account_id - account to retrieve details from
       * @param key - under which keys data should be returned
       * @param writer - from which writers details should be returned
       * @return builder with getAccountDetail query inside
       */
      ModelQueryBuilder getAccountDetail(
          const interface::types::AccountIdType &account_id = "",
          const interface::types::AccountDetailKeyType &key = "",
          const interface::types::AccountIdType &writer = "");

      /**
       * Retrieves all pending (not fully signed) multisignature transactions or
       * batches of transactions.
       * @return builder with getPendingTransactions query inside
       */
      ModelQueryBuilder getPendingTransactions();

      /**
       * Builds result with all appended fields
       * @return wrapper on unsigned query
       */
      proto::UnsignedWrapper<proto::Query> build();

     private:
      proto::TemplateQueryBuilder<
          (1 << shared_model::proto::TemplateQueryBuilder<>::total) - 1>
          builder_;
    };
  }  // namespace bindings
}  // namespace shared_model

#endif  // IROHA_SHARED_MODEL_MODEL_QUERY_BUILDER_HPP

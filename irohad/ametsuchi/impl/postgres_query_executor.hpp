/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_POSTGRES_QUERY_EXECUTOR_HPP
#define IROHA_POSTGRES_QUERY_EXECUTOR_HPP

#include "ametsuchi/query_executor.hpp"

#include "ametsuchi/impl/soci_utils.hpp"
#include "ametsuchi/key_value_storage.hpp"
#include "ametsuchi/storage.hpp"
#include "interfaces/commands/add_asset_quantity.hpp"
#include "interfaces/commands/add_peer.hpp"
#include "interfaces/commands/add_signatory.hpp"
#include "interfaces/commands/append_role.hpp"
#include "interfaces/commands/create_account.hpp"
#include "interfaces/commands/create_asset.hpp"
#include "interfaces/commands/create_domain.hpp"
#include "interfaces/commands/create_role.hpp"
#include "interfaces/commands/detach_role.hpp"
#include "interfaces/commands/grant_permission.hpp"
#include "interfaces/commands/remove_signatory.hpp"
#include "interfaces/commands/revoke_permission.hpp"
#include "interfaces/commands/set_account_detail.hpp"
#include "interfaces/commands/set_quorum.hpp"
#include "interfaces/commands/subtract_asset_quantity.hpp"
#include "interfaces/commands/transfer_asset.hpp"
#include "interfaces/iroha_internal/block_json_converter.hpp"
#include "interfaces/iroha_internal/query_response_factory.hpp"
#include "interfaces/permission_to_string.hpp"
#include "interfaces/queries/blocks_query.hpp"
#include "interfaces/queries/query.hpp"
#include "interfaces/query_responses/query_response.hpp"
#include "logger/logger.hpp"

namespace iroha {
  namespace ametsuchi {

    using QueryErrorType =
        shared_model::interface::QueryResponseFactory::ErrorQueryType;

    class PostgresQueryExecutorVisitor
        : public boost::static_visitor<QueryExecutorResult> {
     public:
      PostgresQueryExecutorVisitor(
          soci::session &sql,
          KeyValueStorage &block_store,
          std::shared_ptr<PendingTransactionStorage> pending_txs_storage,
          std::shared_ptr<shared_model::interface::BlockJsonConverter>
              converter,
          std::shared_ptr<shared_model::interface::QueryResponseFactory>
              response_factory,
          std::shared_ptr<shared_model::interface::PermissionToString>
              perm_converter);

      void setCreatorId(
          const shared_model::interface::types::AccountIdType &creator_id);

      void setQueryHash(const shared_model::crypto::Hash &query_hash);

      QueryExecutorResult operator()(
          const shared_model::interface::GetAccount &q);

      QueryExecutorResult operator()(
          const shared_model::interface::GetSignatories &q);

      QueryExecutorResult operator()(
          const shared_model::interface::GetAccountTransactions &q);

      QueryExecutorResult operator()(
          const shared_model::interface::GetTransactions &q);

      QueryExecutorResult operator()(
          const shared_model::interface::GetAccountAssetTransactions &q);

      QueryExecutorResult operator()(
          const shared_model::interface::GetAccountAssets &q);

      QueryExecutorResult operator()(
          const shared_model::interface::GetAccountDetail &q);

      QueryExecutorResult operator()(
          const shared_model::interface::GetRoles &q);

      QueryExecutorResult operator()(
          const shared_model::interface::GetRolePermissions &q);

      QueryExecutorResult operator()(
          const shared_model::interface::GetAssetInfo &q);

      QueryExecutorResult operator()(
          const shared_model::interface::GetPendingTransactions &q);

     private:
      /**
       * Get transactions from block using range from range_gen and filtered by
       * predicate pred
       */
      template <typename RangeGen, typename Pred>
      std::vector<std::unique_ptr<shared_model::interface::Transaction>>
      getTransactionsFromBlock(uint64_t block_id,
                               RangeGen &&range_gen,
                               Pred &&pred);

      /**
       * Execute query and return its response
       * @tparam QueryTuple - types of values, returned by the query
       * @tparam PermissionTuple - permissions, needed for the query
       * @tparam QueryExecutor - type of function, which executes the query
       * @tparam ResponseCreator - type of function, which creates response of
       * the query
       * @tparam ErrResponse - type of function, which creates error response
       * @param query_executor - function, executing query
       * @param response_creator - function, creating query response
       * @param err_response - function, creating error response
       * @return query response created as a result of query execution
       */
      template <typename QueryTuple,
                typename PermissionTuple,
                typename QueryExecutor,
                typename ResponseCreator,
                typename ErrResponse>
      QueryExecutorResult executeQuery(QueryExecutor &&query_executor,
                                       ResponseCreator &&response_creator,
                                       ErrResponse &&err_response);

      /**
       * Create a query error response and log it
       * @param error_type - type of query error
       * @param error body as string message
       * @return ptr to created error response
       */
      std::unique_ptr<shared_model::interface::QueryResponse>
      logAndReturnErrorResponse(iroha::ametsuchi::QueryErrorType error_type,
                                std::string error_body) const;

      template <typename Query, typename QueryApplier, typename... Permissions>
      QueryExecutorResult executeTransactionsQuery(
          const Query &q,
          const std::string &related_txs,
          QueryApplier applier,
          Permissions... perms);

      soci::session &sql_;
      KeyValueStorage &block_store_;
      shared_model::interface::types::AccountIdType creator_id_;
      shared_model::interface::types::HashType query_hash_;
      std::shared_ptr<PendingTransactionStorage> pending_txs_storage_;
      std::shared_ptr<shared_model::interface::BlockJsonConverter> converter_;
      std::shared_ptr<shared_model::interface::QueryResponseFactory>
          query_response_factory_;
      std::shared_ptr<shared_model::interface::PermissionToString>
          perm_converter_;
      logger::Logger log_;
    };

    class PostgresQueryExecutor : public QueryExecutor {
     public:
      PostgresQueryExecutor(
          std::unique_ptr<soci::session> sql,
          KeyValueStorage &block_store,
          std::shared_ptr<PendingTransactionStorage> pending_txs_storage,
          std::shared_ptr<shared_model::interface::BlockJsonConverter>
              converter,
          std::shared_ptr<shared_model::interface::QueryResponseFactory>
              response_factory,
          std::shared_ptr<shared_model::interface::PermissionToString>
              perm_converter);

      QueryExecutorResult validateAndExecute(
          const shared_model::interface::Query &query) override;

      bool validate(const shared_model::interface::BlocksQuery &query) override;

     private:
      std::unique_ptr<soci::session> sql_;
      KeyValueStorage &block_store_;
      std::shared_ptr<PendingTransactionStorage> pending_txs_storage_;
      PostgresQueryExecutorVisitor visitor_;
      std::shared_ptr<shared_model::interface::QueryResponseFactory>
          query_response_factory_;
      logger::Logger log_;
    };

  }  // namespace ametsuchi
}  // namespace iroha

#endif  // IROHA_POSTGRES_QUERY_EXECUTOR_HPP

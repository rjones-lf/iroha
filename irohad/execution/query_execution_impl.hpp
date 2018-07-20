/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_QUERY_EXECUTION_IMPL_HPP
#define IROHA_QUERY_EXECUTION_IMPL_HPP

#include "execution/query_execution.hpp"

#include "ametsuchi/block_query.hpp"
#include "ametsuchi/storage.hpp"
#include "ametsuchi/wsv_query.hpp"
#include "builders/protobuf/builder_templates/query_response_template.hpp"
#include "interfaces/common_objects/types.hpp"

namespace iroha {

  class QueryExecutionImpl : public QueryExecution {
    using QueryResponseBuilder =
        shared_model::proto::TemplateQueryResponseBuilder<0>;

    using QueryResponseBuilderDone =
        shared_model::proto::TemplateQueryResponseBuilder<1>;

   public:
    QueryExecutionImpl(std::shared_ptr<ametsuchi::Storage> storage);

    std::unique_ptr<shared_model::interface::QueryResponse> validateAndExecute(
        const shared_model::interface::Query &query) override;
    bool validate(const shared_model::interface::BlocksQuery &query) override;

   private:
    bool validate(const shared_model::interface::Query &query,
                  const shared_model::interface::GetAssetInfo &get_asset_info);

    bool validate(const shared_model::interface::Query &query,
                  const shared_model::interface::GetRoles &get_roles);

    bool validate(const shared_model::interface::Query &query,
                  const shared_model::interface::GetRolePermissions
                      &get_role_permissions);

    bool validate(
        const shared_model::interface::Query &query,
        const shared_model::interface::GetAccountAssets &get_account_assets);

    bool validate(const shared_model::interface::Query &query,
                  const shared_model::interface::GetAccount &get_account);

    bool validate(
        const shared_model::interface::Query &query,
        const shared_model::interface::GetSignatories &get_signatories);

    bool validate(const shared_model::interface::Query &query,
                  const shared_model::interface::GetAccountTransactions
                      &get_account_transactions);

    bool validate(const shared_model::interface::Query &query,
                  const shared_model::interface::GetAccountAssetTransactions
                      &get_account_asset_transactions);

    bool validate(
        const shared_model::interface::Query &query,
        const shared_model::interface::GetAccountDetail &get_account_detail);

    bool validate(
        const shared_model::interface::Query &query,
        const shared_model::interface::GetTransactions &get_transactions);

    QueryResponseBuilderDone executeGetAssetInfo(
        const shared_model::interface::GetAssetInfo &get_asset_info);

    QueryResponseBuilderDone executeGetRoles(
        const shared_model::interface::GetRoles &query);

    QueryResponseBuilderDone executeGetRolePermissions(
        const shared_model::interface::GetRolePermissions &query);

    QueryResponseBuilderDone executeGetAccountAssets(
        const shared_model::interface::GetAccountAssets &query);

    QueryResponseBuilderDone executeGetAccountDetail(
        const shared_model::interface::GetAccountDetail &query);

    QueryResponseBuilderDone executeGetAccount(
        const shared_model::interface::GetAccount &query);

    QueryResponseBuilderDone executeGetSignatories(
        const shared_model::interface::GetSignatories &query);

    QueryResponseBuilderDone executeGetAccountAssetTransactions(
        const shared_model::interface::GetAccountAssetTransactions &query);

    QueryResponseBuilderDone executeGetAccountTransactions(
        const shared_model::interface::GetAccountTransactions &query);

    QueryResponseBuilderDone executeGetTransactions(
        const shared_model::interface::GetTransactions &query,
        const shared_model::interface::types::AccountIdType &accountId);

    QueryResponseBuilderDone executeGetPendingTransactions(
        const shared_model::interface::GetPendingTransactions &query,
        const shared_model::interface::types::AccountIdType &query_creator);

    std::shared_ptr<ametsuchi::WsvQuery> wsvQuery_;
    std::shared_ptr<ametsuchi::BlockQuery> blockQuery_;
    std::mutex m;
  };

}  // namespace iroha

#endif  // IROHA_QUERY_EXECUTION_IMPL_HPP

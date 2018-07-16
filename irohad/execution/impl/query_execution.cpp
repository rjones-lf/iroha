/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "execution/query_execution.hpp"

#include <boost/algorithm/string.hpp>

#include "backend/protobuf/permissions.hpp"
#include "execution/common_executor.hpp"

using namespace shared_model::interface::permissions;
using namespace shared_model::proto::permissions;
using namespace iroha;
using namespace iroha::ametsuchi;

QueryProcessingFactory::QueryProcessingFactory(
    std::shared_ptr<ametsuchi::WsvQuery> wsvQuery,
    std::shared_ptr<ametsuchi::BlockQuery> blockQuery)
    : _wsvQuery(std::move(wsvQuery)), _blockQuery(std::move(blockQuery)) {}

std::string getDomainFromName(const std::string &account_id) {
  std::vector<std::string> res;
  boost::split(res, account_id, boost::is_any_of("@"));
  return res.size() > 1 ? res.at(1) : "";
}

/**
 * Generates a query response that contains an error response
 * @tparam T The error to return
 * @param query_hash Query hash
 * @return smart pointer with the QueryResponse
 */
template <class T>
shared_model::proto::TemplateQueryResponseBuilder<1> buildError() {
  return shared_model::proto::TemplateQueryResponseBuilder<0>()
      .errorQueryResponse<T>();
}

/**
 * Generates a query response that contains a concrete error (StatefulFailed)
 * @param query_hash Query hash
 * @return smart pointer with the QueryResponse
 */
shared_model::proto::TemplateQueryResponseBuilder<1> statefulFailed() {
  return buildError<shared_model::interface::StatefulFailedErrorResponse>();
}

static bool hasQueryPermission(const std::string &creator,
                               const std::string &target_account,
                               WsvQuery &wsv_query,
                               Role indiv_permission_id,
                               Role all_permission_id,
                               Role domain_permission_id) {
  auto perms_set = iroha::getAccountPermissions(creator, wsv_query);
  if (not perms_set) {
    return false;
  }

  auto &set = perms_set.value();
  // Creator want to query his account, must have role
  // permission
  return (creator == target_account and set.test(indiv_permission_id)) or
      // Creator has global permission to get any account
      set.test(all_permission_id) or
      // Creator has domain permission
      (getDomainFromName(creator) == getDomainFromName(target_account)
       and set.test(domain_permission_id));
}

bool QueryProcessingFactory::validate(
    const shared_model::interface::BlocksQuery &query) {
  return checkAccountRolePermission(
      query.creatorAccountId(), *_wsvQuery, Role::kGetBlocks);
}
bool QueryProcessingFactory::validate(
    const shared_model::interface::Query &query,
    const shared_model::interface::GetAssetInfo &get_asset_info) {
  return checkAccountRolePermission(
      query.creatorAccountId(), *_wsvQuery, Role::kReadAssets);
}

bool QueryProcessingFactory::validate(
    const shared_model::interface::Query &query,
    const shared_model::interface::GetRoles &get_roles) {
  return checkAccountRolePermission(
      query.creatorAccountId(), *_wsvQuery, Role::kGetRoles);
}

bool QueryProcessingFactory::validate(
    const shared_model::interface::Query &query,
    const shared_model::interface::GetRolePermissions &get_role_permissions) {
  return checkAccountRolePermission(
      query.creatorAccountId(), *_wsvQuery, Role::kGetRoles);
}

bool QueryProcessingFactory::validate(
    const shared_model::interface::Query &query,
    const shared_model::interface::GetAccount &get_account) {
  return hasQueryPermission(query.creatorAccountId(),
                            get_account.accountId(),
                            *_wsvQuery,
                            Role::kGetMyAccount,
                            Role::kGetAllAccounts,
                            Role::kGetDomainAccounts);
}

bool QueryProcessingFactory::validate(
    const shared_model::interface::Query &query,
    const shared_model::interface::GetSignatories &get_signatories) {
  return hasQueryPermission(query.creatorAccountId(),
                            get_signatories.accountId(),
                            *_wsvQuery,
                            Role::kGetMySignatories,
                            Role::kGetAllSignatories,
                            Role::kGetDomainSignatories);
}

bool QueryProcessingFactory::validate(
    const shared_model::interface::Query &query,
    const shared_model::interface::GetAccountAssets &get_account_assets) {
  return hasQueryPermission(query.creatorAccountId(),
                            get_account_assets.accountId(),
                            *_wsvQuery,
                            Role::kGetMyAccAst,
                            Role::kGetAllAccAst,
                            Role::kGetDomainAccAst);
}

bool QueryProcessingFactory::validate(
    const shared_model::interface::Query &query,
    const shared_model::interface::GetAccountDetail &get_account_detail) {
  return hasQueryPermission(query.creatorAccountId(),
                            get_account_detail.accountId(),
                            *_wsvQuery,
                            Role::kGetMyAccDetail,
                            Role::kGetAllAccDetail,
                            Role::kGetDomainAccDetail);
}

bool QueryProcessingFactory::validate(
    const shared_model::interface::Query &query,
    const shared_model::interface::GetAccountTransactions
        &get_account_transactions) {
  return hasQueryPermission(query.creatorAccountId(),
                            get_account_transactions.accountId(),
                            *_wsvQuery,
                            Role::kGetMyAccTxs,
                            Role::kGetAllAccTxs,
                            Role::kGetDomainAccTxs);
}

bool QueryProcessingFactory::validate(
    const shared_model::interface::Query &query,
    const shared_model::interface::GetAccountAssetTransactions
        &get_account_asset_transactions) {
  return hasQueryPermission(query.creatorAccountId(),
                            get_account_asset_transactions.accountId(),
                            *_wsvQuery,
                            Role::kGetMyAccAstTxs,
                            Role::kGetAllAccAstTxs,
                            Role::kGetDomainAccAstTxs);
}

bool QueryProcessingFactory::validate(
    const shared_model::interface::Query &query,
    const shared_model::interface::GetTransactions &get_transactions) {
  return checkAccountRolePermission(
             query.creatorAccountId(), *_wsvQuery, Role::kGetMyTxs)
      or checkAccountRolePermission(
             query.creatorAccountId(), *_wsvQuery, Role::kGetAllTxs);
}

QueryProcessingFactory::QueryResponseBuilderDone
QueryProcessingFactory::executeGetAssetInfo(
    const shared_model::interface::GetAssetInfo &query) {
  auto ast = _wsvQuery->getAsset(query.assetId());

  if (not ast) {
    return buildError<shared_model::interface::NoAssetErrorResponse>();
  }

  const auto &asset = **ast;
  auto response = QueryResponseBuilder().assetResponse(
      asset.assetId(), asset.domainId(), asset.precision());
  return response;
}

QueryProcessingFactory::QueryResponseBuilderDone
QueryProcessingFactory::executeGetRoles(
    const shared_model::interface::GetRoles &queryQueryResponseBuilder) {
  auto roles = _wsvQuery->getRoles();
  if (not roles) {
    return buildError<shared_model::interface::NoRolesErrorResponse>();
  }
  auto response = QueryResponseBuilder().rolesResponse(*roles);
  return response;
}

QueryProcessingFactory::QueryResponseBuilderDone
QueryProcessingFactory::executeGetRolePermissions(
    const shared_model::interface::GetRolePermissions &query) {
  auto perm = _wsvQuery->getRolePermissions(query.roleId());
  if (not perm) {
    return buildError<shared_model::interface::NoRolesErrorResponse>();
  }

  auto response = QueryResponseBuilder().rolePermissionsResponse(*perm);
  return response;
}

QueryProcessingFactory::QueryResponseBuilderDone
QueryProcessingFactory::executeGetAccount(
    const shared_model::interface::GetAccount &query) {
  auto acc = _wsvQuery->getAccount(query.accountId());

  auto roles = _wsvQuery->getAccountRoles(query.accountId());
  if (not acc or not roles) {
    return buildError<shared_model::interface::NoAccountErrorResponse>();
  }

  auto account = std::static_pointer_cast<shared_model::proto::Account>(*acc);
  auto response = QueryResponseBuilder().accountResponse(*account, *roles);
  return response;
}

QueryProcessingFactory::QueryResponseBuilderDone
QueryProcessingFactory::executeGetAccountAssets(
    const shared_model::interface::GetAccountAssets &query) {
  auto acct_assets = _wsvQuery->getAccountAssets(query.accountId());

  if (not acct_assets) {
    return buildError<shared_model::interface::NoAccountAssetsErrorResponse>();
  }
  std::vector<shared_model::proto::AccountAsset> account_assets;
  for (auto asset : *acct_assets) {
    // TODO: IR-1239 remove static cast when query response builder is updated
    // and accepts interface objects
    account_assets.push_back(
        *std::static_pointer_cast<shared_model::proto::AccountAsset>(asset));
  }
  auto response = QueryResponseBuilder().accountAssetResponse(account_assets);
  return response;
}

QueryProcessingFactory::QueryResponseBuilderDone
QueryProcessingFactory::executeGetAccountDetail(
    const shared_model::interface::GetAccountDetail &query) {
  auto acct_detail =
      _wsvQuery->getAccountDetail(query.accountId(),
                                  query.key() ? *query.key() : "",
                                  query.writer() ? *query.writer() : "");
  if (not acct_detail) {
    return buildError<shared_model::interface::NoAccountDetailErrorResponse>();
  }
  auto response = QueryResponseBuilder().accountDetailResponse(*acct_detail);
  return response;
}

QueryProcessingFactory::QueryResponseBuilderDone
QueryProcessingFactory::executeGetAccountAssetTransactions(
    const shared_model::interface::GetAccountAssetTransactions &query) {
  auto acc_asset_tx = _blockQuery->getAccountAssetTransactions(
      query.accountId(), query.assetId());

  std::vector<shared_model::proto::Transaction> txs;
  acc_asset_tx.subscribe([&](const auto &tx) {
    txs.push_back(
        *std::static_pointer_cast<shared_model::proto::Transaction>(tx));
  });

  auto response = QueryResponseBuilder().transactionsResponse(txs);
  return response;
}

QueryProcessingFactory::QueryResponseBuilderDone
QueryProcessingFactory::executeGetAccountTransactions(
    const shared_model::interface::GetAccountTransactions &query) {
  auto acc_tx = _blockQuery->getAccountTransactions(query.accountId());

  std::vector<shared_model::proto::Transaction> txs;
  acc_tx.subscribe([&](const auto &tx) {
    txs.push_back(
        *std::static_pointer_cast<shared_model::proto::Transaction>(tx));
  });

  auto response = QueryResponseBuilder().transactionsResponse(txs);
  return response;
}

QueryProcessingFactory::QueryResponseBuilderDone
QueryProcessingFactory::executeGetTransactions(
    const shared_model::interface::GetTransactions &q,
    const shared_model::interface::types::AccountIdType &accountId) {
  const std::vector<shared_model::crypto::Hash> &hashes = q.transactionHashes();

  auto transactions = _blockQuery->getTransactions(hashes);

  std::vector<shared_model::proto::Transaction> txs;
  bool can_get_all =
      checkAccountRolePermission(accountId, *_wsvQuery, Role::kGetAllTxs);
  transactions.subscribe([&](const auto &tx) {
    if (tx) {
      auto proto_tx =
          *std::static_pointer_cast<shared_model::proto::Transaction>(*tx);
      if (can_get_all or proto_tx.creatorAccountId() == accountId)
        txs.push_back(proto_tx);
    }
  });

  auto response = QueryResponseBuilder().transactionsResponse(txs);
  return response;
}

QueryProcessingFactory::QueryResponseBuilderDone
QueryProcessingFactory::executeGetSignatories(
    const shared_model::interface::GetSignatories &query) {
  auto signs = _wsvQuery->getSignatories(query.accountId());
  if (not signs) {
    return buildError<shared_model::interface::NoSignatoriesErrorResponse>();
  }
  auto response = QueryResponseBuilder().signatoriesResponse(*signs);
  return response;
}

QueryProcessingFactory::QueryResponseBuilderDone
QueryProcessingFactory::executeGetPendingTransactions(
    const shared_model::interface::GetPendingTransactions &query,
    const shared_model::interface::types::AccountIdType &query_creator) {
  std::vector<shared_model::proto::Transaction> txs;
  // TODO 2018-07-04, igor-egorov, IR-1486, the core logic is to be implemented
  auto response = QueryResponseBuilder().transactionsResponse(txs);
  return response;
}

std::shared_ptr<shared_model::interface::QueryResponse>
QueryProcessingFactory::validateAndExecute(
    const shared_model::interface::Query &query) {
  const auto &query_hash = query.hash();
  QueryResponseBuilderDone builder;
  // TODO: 29/04/2018 x3medima18, Add visitor class, IR-1185
  return visit_in_place(
      query.get(),
      [&](const shared_model::interface::GetAccount &q) {
        if (not validate(query, q)) {
          builder = statefulFailed();
        } else {
          builder = executeGetAccount(q);
        }
        return clone(builder.queryHash(query_hash).build());
      },
      [&](const shared_model::interface::GetSignatories &q) {
        if (not validate(query, q)) {
          builder = statefulFailed();
        } else {
          builder = executeGetSignatories(q);
        }
        return clone(builder.queryHash(query_hash).build());
      },
      [&](const shared_model::interface::GetAccountTransactions &q) {
        if (not validate(query, q)) {
          builder = statefulFailed();
        } else {
          builder = executeGetAccountTransactions(q);
        }
        return clone(builder.queryHash(query_hash).build());
      },
      [&](const shared_model::interface::GetTransactions &q) {
        if (not validate(query, q)) {
          builder = statefulFailed();
        } else {
          builder = executeGetTransactions(q, query.creatorAccountId());
        }
        return clone(builder.queryHash(query_hash).build());
      },
      [&](const shared_model::interface::GetAccountAssetTransactions &q) {
        if (not validate(query, q)) {
          builder = statefulFailed();
        } else {
          builder = executeGetAccountAssetTransactions(q);
        }
        return clone(builder.queryHash(query_hash).build());
      },
      [&](const shared_model::interface::GetAccountAssets &q) {
        if (not validate(query, q)) {
          builder = statefulFailed();
        } else {
          builder = executeGetAccountAssets(q);
        }
        return clone(builder.queryHash(query_hash).build());
      },
      [&](const shared_model::interface::GetAccountDetail &q) {
        if (not validate(query, q)) {
          builder = statefulFailed();
        } else {
          builder = executeGetAccountDetail(q);
        }
        return clone(builder.queryHash(query_hash).build());
      },
      [&](const shared_model::interface::GetRoles &q) {
        if (not validate(query, q)) {
          builder = statefulFailed();
        } else {
          builder = executeGetRoles(q);
        }
        return clone(builder.queryHash(query_hash).build());
      },
      [&](const shared_model::interface::GetRolePermissions &q) {
        if (not validate(query, q)) {
          builder = statefulFailed();
        } else {
          builder = executeGetRolePermissions(q);
        }
        return clone(builder.queryHash(query_hash).build());
      },
      [&](const shared_model::interface::GetAssetInfo &q) {
        if (not validate(query, q)) {
          builder = statefulFailed();
        } else {
          builder = executeGetAssetInfo(q);
        }
        return clone(builder.queryHash(query_hash).build());
      },
      [&](const shared_model::interface::GetPendingTransactions &q) {
        // the query does not require validation
        builder = executeGetPendingTransactions(q, query.creatorAccountId());
        return clone(builder.queryHash(query_hash).build());
      }

  );
}

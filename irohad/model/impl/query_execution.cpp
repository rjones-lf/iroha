/**
 * Copyright Soramitsu Co., Ltd. 2017 All Rights Reserved.
 * http://soramitsu.co.jp
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "model/query_execution.hpp"

#include <boost/algorithm/string.hpp>

#include "execution/common_executor.hpp"
#include "validators/permissions.hpp"

using namespace iroha::model;
using namespace shared_model::permissions;
using namespace iroha::ametsuchi;

// TODO: 28/03/2018 x3medima17 remove poly wrapper, IR-1011
template <class T>
using w = shared_model::detail::PolymorphicWrapper<T>;

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

bool hasQueryPermission(const std::string &creator,
                        const std::string &target_account,
                        WsvQuery &wsv_query,
                        const std::string &indiv_permission_id,
                        const std::string &all_permission_id,
                        const std::string &domain_permission_id) {
  auto perms_set = iroha::getAccountPermissions(creator, wsv_query);
  return
      // 1. Creator has grant permission from other user
      (creator != target_account
       and wsv_query.hasAccountGrantablePermission(
               creator, target_account, indiv_permission_id))
      or  // ----- Creator has role permission ---------
      (perms_set
       and (
               // 2. Creator want to query his account, must have role
               // permission
               (creator == target_account
                and iroha::accountHasPermission(perms_set.value(),
                                                indiv_permission_id))
               or  // 3. Creator has global permission to get any account
               (iroha::accountHasPermission(perms_set.value(),
                                            all_permission_id))
               or  // 4. Creator has domain permission
               (getDomainFromName(creator) == getDomainFromName(target_account)
                and iroha::accountHasPermission(perms_set.value(),
                                                domain_permission_id))));
}

bool QueryProcessingFactory::validate(
    const shared_model::interface::Query &query,
    const shared_model::interface::GetAssetInfo &get_asset_info) {
  // TODO: 03.02.2018 grimadas IR-851: check signatures
  return checkAccountRolePermission(
      query.creatorAccountId(), *_wsvQuery, can_read_assets);
}

bool QueryProcessingFactory::validate(
    const shared_model::interface::Query &query,
    const shared_model::interface::GetRoles &get_roles) {
  // TODO: 03.02.2018 grimadas IR-851: check signatures
  return checkAccountRolePermission(
      query.creatorAccountId(), *_wsvQuery, can_get_roles);
}

bool QueryProcessingFactory::validate(
    const shared_model::interface::Query &query,
    const shared_model::interface::GetRolePermissions &get_role_permissions) {
  // TODO: 03.02.2018 grimadas IR-851: check signatures
  return checkAccountRolePermission(
      query.creatorAccountId(), *_wsvQuery, can_get_roles);
}

bool QueryProcessingFactory::validate(
    const shared_model::interface::Query &query,
    const shared_model::interface::GetAccount &get_account) {
  // TODO: 03.02.2018 grimadas IR-851: check signatures
  return hasQueryPermission(query.creatorAccountId(),
                            get_account.accountId(),
                            *_wsvQuery,
                            can_get_my_account,
                            can_get_all_accounts,
                            can_get_domain_accounts);
}

bool QueryProcessingFactory::validate(
    const shared_model::interface::Query &query,
    const shared_model::interface::GetSignatories &get_signatories) {
  // TODO: 03.02.2018 grimadas IR-851: check signatures
  return hasQueryPermission(query.creatorAccountId(),
                            get_signatories.accountId(),
                            *_wsvQuery,
                            can_get_my_signatories,
                            can_get_all_signatories,
                            can_get_domain_signatories);
}

bool QueryProcessingFactory::validate(
    const shared_model::interface::Query &query,
    const shared_model::interface::GetAccountAssets &get_account_assets) {
  // TODO: 03.02.2018 grimadas IR-851: check signatures
  return hasQueryPermission(query.creatorAccountId(),
                            get_account_assets.accountId(),
                            *_wsvQuery,
                            can_get_my_acc_ast,
                            can_get_all_acc_ast,
                            can_get_domain_acc_ast);
}

bool QueryProcessingFactory::validate(
    const shared_model::interface::Query &query,
    const shared_model::interface::GetAccountDetail &get_account_detail) {
  // TODO: 03.02.2018 grimadas IR-851: check signatures
  return hasQueryPermission(query.creatorAccountId(),
                            get_account_detail.accountId(),
                            *_wsvQuery,
                            can_get_my_acc_detail,
                            can_get_all_acc_detail,
                            can_get_domain_acc_detail);
}

bool QueryProcessingFactory::validate(
    const shared_model::interface::Query &query,
    const shared_model::interface::GetAccountTransactions
        &get_account_transactions) {
  // TODO: 03.02.2018 grimadas IR-851: check signatures
  return hasQueryPermission(query.creatorAccountId(),
                            get_account_transactions.accountId(),
                            *_wsvQuery,
                            can_get_my_acc_txs,
                            can_get_all_acc_txs,
                            can_get_domain_acc_txs);
}

bool QueryProcessingFactory::validate(
    const shared_model::interface::Query &query,
    const shared_model::interface::GetAccountAssetTransactions
        &get_account_asset_transactions) {
  // TODO: 03.02.2018 grimadas IR-851: check signatures
  return hasQueryPermission(query.creatorAccountId(),
                            get_account_asset_transactions.accountId(),
                            *_wsvQuery,
                            can_get_my_acc_ast_txs,
                            can_get_all_acc_ast_txs,
                            can_get_domain_acc_ast_txs);
}

bool QueryProcessingFactory::validate(
    const shared_model::interface::Query &query,
    const shared_model::interface::GetTransactions &get_transactions) {
  // TODO: 03.02.2018 grimadas IR-851: check signatures
  return checkAccountRolePermission(
             query.creatorAccountId(), *_wsvQuery, can_get_my_txs)
      or checkAccountRolePermission(
             query.creatorAccountId(), *_wsvQuery, can_get_all_txs);
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
  auto acct_asset =
      _wsvQuery->getAccountAsset(query.accountId(), query.assetId());

  if (not acct_asset) {
    return buildError<shared_model::interface::NoAccountAssetsErrorResponse>();
  }

  const auto &account_asset = **acct_asset;
  auto response =
      QueryResponseBuilder().accountAssetResponse(account_asset.assetId(),
                                                  account_asset.accountId(),
                                                  account_asset.balance());
  return response;
}

QueryProcessingFactory::QueryResponseBuilderDone
iroha::model::QueryProcessingFactory::executeGetAccountDetail(
    const shared_model::interface::GetAccountDetail &query) {
  auto acct_detail = _wsvQuery->getAccountDetail(query.accountId());
  if (not acct_detail) {
    return buildError<shared_model::interface::NoAccountDetailErrorResponse>();
  }
  auto response = QueryResponseBuilder().accountDetailResponse(*acct_detail);
  return response;
}

QueryProcessingFactory::QueryResponseBuilderDone
iroha::model::QueryProcessingFactory::executeGetAccountAssetTransactions(
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
iroha::model::QueryProcessingFactory::executeGetTransactions(
    const shared_model::interface::GetTransactions &q,
    const shared_model::interface::types::AccountIdType &accountId) {
  const std::vector<shared_model::crypto::Hash> &hashes = q.transactionHashes();

  auto transactions = _blockQuery->getTransactions(hashes);

  std::vector<shared_model::proto::Transaction> txs;
  bool can_get_all =
      checkAccountRolePermission(accountId, *_wsvQuery, can_get_all_txs);
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

std::shared_ptr<shared_model::interface::QueryResponse>
QueryProcessingFactory::execute(const shared_model::interface::Query &query) {
  const auto &query_hash = query.hash();
  QueryResponseBuilderDone builder;
  // TODO: 29/04/2018 x3medima18, Add visitor class, IR-1185
  return visit_in_place(
      query.get(),
      [&](const w<shared_model::interface::GetAccount> &q) {
        if (not validate(query, *q)) {
          builder = statefulFailed();
        } else {
          builder = executeGetAccount(*q);
        }
        return clone(builder.queryHash(query_hash).build());
      },
      [&](const w<shared_model::interface::GetSignatories> &q) {
        if (not validate(query, *q)) {
          builder = statefulFailed();
        } else {
          builder = executeGetSignatories(*q);
        }
        return clone(builder.queryHash(query_hash).build());
      },
      [&](const w<shared_model::interface::GetAccountTransactions> &q) {
        if (not validate(query, *q)) {
          builder = statefulFailed();
        } else {
          builder = executeGetAccountTransactions(*q);
        }
        return clone(builder.queryHash(query_hash).build());
      },
      [&](const w<shared_model::interface::GetTransactions> &q) {
        if (not validate(query, *q)) {
          builder = statefulFailed();
        } else {
          builder = executeGetTransactions(*q, query.creatorAccountId());
        }
        return clone(builder.queryHash(query_hash).build());
      },
      [&](const w<shared_model::interface::GetAccountAssetTransactions> &q) {
        if (not validate(query, *q)) {
          builder = statefulFailed();
        } else {
          builder = executeGetAccountAssetTransactions(*q);
        }
        return clone(builder.queryHash(query_hash).build());
      },
      [&](const w<shared_model::interface::GetAccountAssets> &q) {
        if (not validate(query, *q)) {
          builder = statefulFailed();
        } else {
          builder = executeGetAccountAssets(*q);
        }
        return clone(builder.queryHash(query_hash).build());
      },
      [&](const w<shared_model::interface::GetAccountDetail> &q) {
        if (not validate(query, *q)) {
          builder = statefulFailed();
        } else {
          builder = executeGetAccountDetail(*q);
        }
        return clone(builder.queryHash(query_hash).build());
      },
      [&](const w<shared_model::interface::GetRoles> &q) {
        if (not validate(query, *q)) {
          builder = statefulFailed();
        } else {
          builder = executeGetRoles(*q);
        }
        return clone(builder.queryHash(query_hash).build());
      },
      [&](const w<shared_model::interface::GetRolePermissions> &q) {
        if (not validate(query, *q)) {
          builder = statefulFailed();
        } else {
          builder = executeGetRolePermissions(*q);
        }
        return clone(builder.queryHash(query_hash).build());
      },
      [&](const w<shared_model::interface::GetAssetInfo> &q) {
        if (not validate(query, *q)) {
          builder = statefulFailed();
        } else {
          builder = executeGetAssetInfo(*q);
        }
        return clone(builder.queryHash(query_hash).build());
      }

  );
}

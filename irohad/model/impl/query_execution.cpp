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

#include "crypto/hash.hpp"
#include "model/query_execution.hpp"
#include "model/execution/common_executor.hpp"
#include "model/permissions.hpp"
#include "model/queries/responses/account_assets_response.hpp"
#include "model/queries/responses/account_response.hpp"
#include "model/queries/responses/asset_response.hpp"
#include "model/queries/responses/error_response.hpp"
#include "model/queries/responses/roles_response.hpp"
#include "model/queries/responses/signatories_response.hpp"
#include "model/queries/responses/transactions_response.hpp"

using namespace iroha::model;
using namespace iroha::ametsuchi;

iroha::model::QueryProcessingFactory::QueryProcessingFactory(
    std::shared_ptr<ametsuchi::WsvQuery> wsvQuery,
    std::shared_ptr<ametsuchi::BlockQuery> blockQuery)
    : _wsvQuery(wsvQuery), _blockQuery(blockQuery) {}

bool hasQueryPermission(const std::string& creator,
                        const std::string& target_account, WsvQuery& wsv_query,
                        const std::string& indiv_permission_id,
                        const std::string& all_permission_id) {
  return
      // 1. Creator want to query his account, must have role permission on that
      (creator == target_account
       and checkAccountRolePermission(creator, wsv_query,
                                      indiv_permission_id))
      or  // 2. Creator has permission to get any account
      (checkAccountRolePermission(creator, wsv_query, all_permission_id))
      or  // 3. Creator has grant permission
      (creator != target_account
       and wsv_query.hasAccountGrantablePermission(creator, target_account,
                                                   indiv_permission_id));
  // TODO: add domain permissions
}

bool iroha::model::QueryProcessingFactory::validate(
    const model::GetAssetInfo& query) {
  // TODO: check signatures
  // TODO: add can read assets in domain
  return checkAccountRolePermission(query.creator_account_id, *_wsvQuery,
                                    can_read_assets);
}

bool iroha::model::QueryProcessingFactory::validate(
    const model::GetRoles& query) {
  // TODO: check signatures
  return checkAccountRolePermission(query.creator_account_id, *_wsvQuery,
                                    can_get_roles);
}

bool QueryProcessingFactory::validate(const model::GetRolePermissions& query) {
  // TODO: check signatures
  return checkAccountRolePermission(query.creator_account_id, *_wsvQuery,
                                    can_get_roles);
}

bool iroha::model::QueryProcessingFactory::validate(
    const model::GetAccount& query) {
  // TODO: check signatures
  return hasQueryPermission(query.creator_account_id, query.account_id,
                            *_wsvQuery, can_get_my_account,
                            can_get_all_accounts);
}

bool iroha::model::QueryProcessingFactory::validate(
    const model::GetSignatories& query) {
  // TODO: check signatures
  return
      hasQueryPermission(query.creator_account_id, query.account_id, *_wsvQuery,
                         can_get_my_signatories, can_get_all_signatories);
}

bool iroha::model::QueryProcessingFactory::validate(
    const model::GetAccountAssets& query) {
  // TODO: check signatures
  return
      hasQueryPermission(query.creator_account_id, query.account_id, *_wsvQuery,
                         can_get_my_acc_ast, can_get_all_acc_ast);
}

bool iroha::model::QueryProcessingFactory::validate(
    const model::GetAccountTransactions& query) {
  // TODO: check signatures
  return
      hasQueryPermission(query.creator_account_id, query.account_id, *_wsvQuery,
                         can_get_my_acc_txs, can_get_all_acc_txs);
}

bool iroha::model::QueryProcessingFactory::validate(
    const model::GetAccountAssetTransactions& query) {
  // TODO: check signatures
  return
      hasQueryPermission(query.creator_account_id, query.account_id, *_wsvQuery,
                         can_get_my_acc_ast_txs, can_get_all_acc_ast_txs);
}

std::shared_ptr<iroha::model::QueryResponse>
QueryProcessingFactory::executeGetAssetInfo(const model::GetAssetInfo& query) {
  auto ast = _wsvQuery->getAsset(query.asset_id);
  if (!ast.has_value()) {
    iroha::model::ErrorResponse response;
    response.query_hash = iroha::hash(query);
    response.reason = iroha::model::ErrorResponse::NO_ASSET;
    return std::make_shared<ErrorResponse>(response);
  }
  AssetResponse response;
  response.asset = std::move(ast.value());
  response.query_hash = iroha::hash(query);
  return std::make_shared<AssetResponse>(response);
}

std::shared_ptr<iroha::model::QueryResponse>
QueryProcessingFactory::executeGetRoles(const model::GetRoles& query) {
  auto roles = _wsvQuery->getRoles();
  if (not roles.has_value()) {
    ErrorResponse response;
    response.query_hash = iroha::hash(query);
    response.reason = ErrorResponse::NO_ROLES;
    return std::make_shared<ErrorResponse>(response);
  }
  RolesResponse response;
  response.query_hash = iroha::hash(query);
  response.roles = std::move(roles.value());
  return std::make_shared<RolesResponse>(response);
}

std::shared_ptr<iroha::model::QueryResponse>
QueryProcessingFactory::executeGetRolePermissions(
    const model::GetRolePermissions& query) {
  auto perm = _wsvQuery->getRolePermissions(query.role_id);
  if (not perm.has_value()) {
    ErrorResponse response;
    response.query_hash = iroha::hash(query);
    response.reason = ErrorResponse::NO_ROLES;
    return std::make_shared<ErrorResponse>(response);
  }
  RolePermissionsResponse response;
  response.query_hash = iroha::hash(query);
  response.role_permissions = std::move(perm.value());
  return std::make_shared<RolePermissionsResponse>(response);
}

std::shared_ptr<iroha::model::QueryResponse>
iroha::model::QueryProcessingFactory::executeGetAccount(
    const model::GetAccount& query) {
  auto acc = _wsvQuery->getAccount(query.account_id);
  auto roles = _wsvQuery->getAccountRoles(query.account_id);
  if (not acc.has_value() or not roles.has_value()) {
    iroha::model::ErrorResponse response;
    response.query_hash = iroha::hash(query);
    response.reason = iroha::model::ErrorResponse::NO_ACCOUNT;
    return std::make_shared<ErrorResponse>(response);
  }

  iroha::model::AccountResponse response;
  response.account = acc.value();
  response.roles = roles.value();
  response.query_hash = iroha::hash(query);
  return std::make_shared<iroha::model::AccountResponse>(response);
}

std::shared_ptr<iroha::model::QueryResponse>
iroha::model::QueryProcessingFactory::executeGetAccountAssets(
    const model::GetAccountAssets& query) {
  auto acct_asset =
      _wsvQuery->getAccountAsset(query.account_id, query.asset_id);
  if (!acct_asset.has_value()) {
    iroha::model::ErrorResponse response;
    response.query_hash = iroha::hash(query);
    response.reason = iroha::model::ErrorResponse::NO_ACCOUNT_ASSETS;
    return std::make_shared<iroha::model::ErrorResponse>(response);
  }
  iroha::model::AccountAssetResponse response;
  response.acct_asset = acct_asset.value();
  response.query_hash = iroha::hash(query);
  return std::make_shared<iroha::model::AccountAssetResponse>(response);
}

std::shared_ptr<iroha::model::QueryResponse>
iroha::model::QueryProcessingFactory::executeGetAccountAssetTransactions(
    const model::GetAccountAssetTransactions& query) {
  auto acc_asset_tx = _blockQuery->getAccountAssetTransactions(query.account_id,
                                                               query.asset_id);
  iroha::model::TransactionsResponse response;
  response.query_hash = iroha::hash(query);
  response.transactions = acc_asset_tx;
  return std::make_shared<iroha::model::TransactionsResponse>(response);
}

std::shared_ptr<iroha::model::QueryResponse>
iroha::model::QueryProcessingFactory::executeGetAccountTransactions(
    const model::GetAccountTransactions& query) {
  auto acc_tx = _blockQuery->getAccountTransactions(
    query.account_id, query.pager);
  iroha::model::TransactionsResponse response;
  response.query_hash = iroha::hash(query);
  response.transactions = acc_tx;
  return std::make_shared<iroha::model::TransactionsResponse>(response);
}

std::shared_ptr<iroha::model::QueryResponse>
iroha::model::QueryProcessingFactory::executeGetSignatories(
    const model::GetSignatories& query) {
  auto signs = _wsvQuery->getSignatories(query.account_id);
  if (!signs.has_value()) {
    iroha::model::ErrorResponse response;
    response.query_hash = iroha::hash(query);
    response.reason = model::ErrorResponse::NO_SIGNATORIES;
    return std::make_shared<iroha::model::ErrorResponse>(response);
  }
  iroha::model::SignatoriesResponse response;
  response.query_hash = iroha::hash(query);
  response.keys = signs.value();
  return std::make_shared<iroha::model::SignatoriesResponse>(response);
}

std::shared_ptr<iroha::model::QueryResponse>
iroha::model::QueryProcessingFactory::execute(
    std::shared_ptr<const model::Query> query) {
  // TODO 26/09/17 Nasrulin: change to handler map or/with templates #VARIANT
  if (instanceof <iroha::model::GetAccount>(query.get())) {
    auto qry = std::static_pointer_cast<const iroha::model::GetAccount>(query);

    if (!validate(*qry)) {
      iroha::model::ErrorResponse response;
      response.query_hash = iroha::hash(*qry);
      response.reason = model::ErrorResponse::STATEFUL_INVALID;
      return std::make_shared<ErrorResponse>(response);
    }
    return executeGetAccount(*qry);
  }
  if (instanceof <iroha::model::GetAccountAssets>(query.get())) {
    auto qry =
        std::static_pointer_cast<const iroha::model::GetAccountAssets>(query);
    if (!validate(*qry)) {
      iroha::model::ErrorResponse response;
      response.query_hash = iroha::hash(*qry);
      response.reason = model::ErrorResponse::STATEFUL_INVALID;
      return std::make_shared<iroha::model::ErrorResponse>(response);
    }
    return executeGetAccountAssets(*qry);
  }
  if (instanceof <iroha::model::GetSignatories>(query.get())) {
    auto qry =
        std::static_pointer_cast<const iroha::model::GetSignatories>(query);
    if (!validate(*qry)) {
      iroha::model::ErrorResponse response;
      response.query_hash = iroha::hash(*qry);
      response.reason = model::ErrorResponse::STATEFUL_INVALID;
      return std::make_shared<iroha::model::ErrorResponse>(response);
    }
    return executeGetSignatories(*qry);
  }
  if (instanceof <iroha::model::GetAccountTransactions>(query.get())) {
    auto qry =
        std::static_pointer_cast<const iroha::model::GetAccountTransactions>(
            query);
    if (!validate(*qry)) {
      iroha::model::ErrorResponse response;
      response.query_hash = iroha::hash(*qry);
      response.reason = model::ErrorResponse::STATEFUL_INVALID;
      return std::make_shared<iroha::model::ErrorResponse>(response);
    }
    return executeGetAccountTransactions(*qry);
  }
  if (instanceof <iroha::model::GetAccountAssetTransactions>(query.get())) {
    auto qry = std::static_pointer_cast<
        const iroha::model::GetAccountAssetTransactions>(query);
    if (!validate(*qry)) {
      iroha::model::ErrorResponse response;
      response.query_hash = iroha::hash(*qry);
      response.reason = model::ErrorResponse::STATEFUL_INVALID;
      return std::make_shared<iroha::model::ErrorResponse>(response);
    }
    return executeGetAccountAssetTransactions(*qry);
  }
  if (instanceof <GetRoles>(query.get())) {
    auto qry = std::static_pointer_cast<const GetRoles>(query);
    if (not validate(*qry)) {
      ErrorResponse response;
      response.query_hash = iroha::hash(*qry);
      response.reason = ErrorResponse::STATEFUL_INVALID;
      return std::make_shared<ErrorResponse>(response);
    }
    return executeGetRoles(*qry);
  }
  if (instanceof <GetRolePermissions>(query.get())) {
    auto qry = std::static_pointer_cast<const GetRolePermissions>(query);
    if (not validate(*qry)) {
      ErrorResponse response;
      response.query_hash = iroha::hash(*qry);
      response.reason = ErrorResponse::STATEFUL_INVALID;
      return std::make_shared<ErrorResponse>(response);
    }
    return executeGetRolePermissions(*qry);
  }
  if (instanceof <GetAssetInfo>(query.get())) {
    auto qry = std::static_pointer_cast<const GetAssetInfo>(query);
    if (not validate(*qry)) {
      ErrorResponse response;
      response.query_hash = iroha::hash(*qry);
      response.reason = ErrorResponse::STATEFUL_INVALID;
      return std::make_shared<ErrorResponse>(response);
    }
    return executeGetAssetInfo(*qry);
  }
  iroha::model::ErrorResponse response;
  response.query_hash = iroha::hash(*query);
  response.reason = model::ErrorResponse::NOT_SUPPORTED;
  return std::make_shared<iroha::model::ErrorResponse>(response);
}

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
#include "crypto/hash.hpp"
#include "model/queries/responses/account_assets_response.hpp"
#include "model/queries/responses/account_response.hpp"
#include "model/queries/responses/asset_response.hpp"
#include "model/queries/responses/error_response.hpp"
#include "model/queries/responses/roles_response.hpp"
#include "model/queries/responses/signatories_response.hpp"
#include "model/queries/responses/transactions_response.hpp"

using namespace iroha::model;

iroha::model::QueryProcessingFactory::QueryProcessingFactory(
    std::shared_ptr<ametsuchi::WsvQuery> wsvQuery,
    std::shared_ptr<ametsuchi::BlockQuery> blockQuery)
    : _wsvQuery(wsvQuery), _blockQuery(blockQuery) {}

bool iroha::model::QueryProcessingFactory::validate(
    const model::GetAssetInfo &query) {
  auto creator = _wsvQuery->getAccount(query.creator_account_id);
  // TODO: check signatures
  return
      // Creator account exits
      // TODO: add permission check
      creator.has_value();
}

bool iroha::model::QueryProcessingFactory::validate(
    const model::GetRoles &query) {
  auto creator = _wsvQuery->getAccount(query.creator_account_id);
  // TODO: check signatures
  return
      // Creator account exits
      // TODO: add permission check
      creator.has_value();
}

bool QueryProcessingFactory::validate(const model::GetRolePermissions &query) {
  auto creator = _wsvQuery->getAccount(query.creator_account_id);
  // TODO: check signatures
  return
      // Creator account exits
      // TODO: add permission check
      creator.has_value();
}

bool iroha::model::QueryProcessingFactory::validate(
    const model::GetAccount &query) {
  auto creator = _wsvQuery->getAccount(query.creator_account_id);
  // TODO: check signatures
  return
      // Creator account exits
      creator.has_value() &&
      // Creator has permission to read, or account = creator
      (creator.value().permissions.read_all_accounts
       || query.account_id == query.creator_account_id);
}

bool iroha::model::QueryProcessingFactory::validate(
    const model::GetSignatories &query) {
  auto creator = _wsvQuery->getAccount(query.creator_account_id);
  return
      // Creator account exits
      creator.has_value() &&
      // Creator has permission to read, or account = creator
      (creator.value().permissions.read_all_accounts
       || query.account_id == query.creator_account_id);
}

bool iroha::model::QueryProcessingFactory::validate(
    const model::GetAccountAssets &query) {
  auto creator = _wsvQuery->getAccount(query.creator_account_id);
  return
      // Creator account exits
      creator.has_value() &&
      // Creator has permission to read, or account = creator
      (creator.value().permissions.read_all_accounts
       || query.account_id == query.creator_account_id);
}

bool iroha::model::QueryProcessingFactory::validate(
    const model::GetAccountTransactions &query) {
  auto creator = _wsvQuery->getAccount(query.creator_account_id);
  return
      // Creator account exits
      creator.has_value() &&
      // Creator has permission to read, or account = creator
      (creator.value().permissions.read_all_accounts
       || query.account_id == query.creator_account_id);
}

bool iroha::model::QueryProcessingFactory::validate(
    const model::GetAccountAssetTransactions &query) {
  auto creator = _wsvQuery->getAccount(query.creator_account_id);
  return
      // Creator account exits
      creator.has_value() &&
      // Creator has permission to read, or account = creator
      (creator.value().permissions.read_all_accounts
       || query.account_id == query.creator_account_id);
}

bool iroha::model::QueryProcessingFactory::validate(
    const model::GetAccountTransactionsWithPager &query) {
  auto creator = _wsvQuery->getAccount(query.creator_account_id);
  return
      // Creator account exits
      creator.has_value() &&
      // Creator has permission to read, or account = creator
      (creator.value().permissions.read_all_accounts
       || query.account_id == query.creator_account_id);
}

bool iroha::model::QueryProcessingFactory::validate(
    const model::GetAccountAssetsTransactionsWithPager &query) {
  auto creator = _wsvQuery->getAccount(query.creator_account_id);
  return
      // Creator account exits
      creator.has_value() &&
      // Creator has permission to read, or account = creator
      (creator.value().permissions.read_all_accounts
       || query.account_id == query.creator_account_id);
}

std::shared_ptr<iroha::model::QueryResponse>
QueryProcessingFactory::executeGetAssetInfo(const model::GetAssetInfo &query) {
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
QueryProcessingFactory::executeGetRoles(const model::GetRoles &query) {
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
    const model::GetRolePermissions &query) {
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
    const model::GetAccount &query) {
  auto acc = _wsvQuery->getAccount(query.account_id);
  if (!acc.has_value()) {
    iroha::model::ErrorResponse response;
    response.query_hash = iroha::hash(query);
    response.reason = iroha::model::ErrorResponse::NO_ACCOUNT;
    return std::make_shared<ErrorResponse>(response);
  }
  iroha::model::AccountResponse response;
  response.account = acc.value();
  response.query_hash = iroha::hash(query);
  return std::make_shared<iroha::model::AccountResponse>(response);
}

std::shared_ptr<iroha::model::QueryResponse>
iroha::model::QueryProcessingFactory::executeGetAccountAssets(
    const model::GetAccountAssets &query) {
  auto acct_asset =
      _wsvQuery->getAccountAsset(query.account_id, query.asset_id);
  if (!acct_asset.has_value()) {
    iroha::model::ErrorResponse response;
    response.query_hash = iroha::hash(query);
    response.reason = iroha::model::ErrorResponse::NO_ACCOUNT_ASSETS;
    return std::make_shared<iroha::model::ErrorResponse>(response);
  }
  auto asset = _wsvQuery->getAsset(query.asset_id);
  // TODO: Add format with precision balance
  iroha::model::AccountAssetResponse response;
  response.acct_asset = acct_asset.value();
  response.query_hash = iroha::hash(query);
  return std::make_shared<iroha::model::AccountAssetResponse>(response);
}

std::shared_ptr<iroha::model::QueryResponse>
iroha::model::QueryProcessingFactory::executeGetAccountAssetTransactions(
    const model::GetAccountAssetTransactions &query) {
  auto acc_asset_tx = _blockQuery->getAccountAssetTransactions(query.account_id,
                                                               query.asset_id);
  iroha::model::TransactionsResponse response;
  response.query_hash = iroha::hash(query);
  response.transactions = acc_asset_tx;
  return std::make_shared<iroha::model::TransactionsResponse>(response);
}

std::shared_ptr<iroha::model::QueryResponse>
iroha::model::QueryProcessingFactory::executeGetAccountTransactions(
    const model::GetAccountTransactions &query) {
  auto acc_tx = _blockQuery->getAccountTransactions(query.account_id);
  iroha::model::TransactionsResponse response;
  response.query_hash = iroha::hash(query);
  response.transactions = acc_tx;
  return std::make_shared<iroha::model::TransactionsResponse>(response);
}

std::shared_ptr<iroha::model::QueryResponse> iroha::model::
    QueryProcessingFactory::executeGetAccountAssetsTransactionsWithPager(
        const model::GetAccountAssetsTransactionsWithPager &query) {
  auto acc_asset_tx = _blockQuery->getAccountAssetsTransactionsWithPager(
      query.account_id, query.assets_id, query.pager);
  iroha::model::TransactionsResponse response;
  response.query_hash = iroha::hash(query);
  response.transactions = acc_asset_tx;
  return std::make_shared<iroha::model::TransactionsResponse>(response);
}

std::shared_ptr<iroha::model::QueryResponse>
iroha::model::QueryProcessingFactory::executeGetAccountTransactionsWithPager(
    const model::GetAccountTransactionsWithPager &query) {
  auto acc_tx = _blockQuery->getAccountTransactionsWithPager(
      query.account_id, query.pager);
  iroha::model::TransactionsResponse response;
  response.query_hash = iroha::hash(query);
  response.transactions = acc_tx;
  return std::make_shared<iroha::model::TransactionsResponse>(response);
}

std::shared_ptr<iroha::model::QueryResponse>
iroha::model::QueryProcessingFactory::executeGetSignatories(
    const model::GetSignatories &query) {
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
  // TODO: change to handler map or/with templates
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
  if (instanceof <iroha::model::GetAccountTransactionsWithPager>(query.get())) {
    auto qry =
      std::static_pointer_cast<const iroha::model::GetAccountTransactionsWithPager>(
        query);
    if (!validate(*qry)) {
      iroha::model::ErrorResponse response;
      response.query_hash = iroha::hash(*qry);
      response.reason = model::ErrorResponse::STATEFUL_INVALID;
      return std::make_shared<iroha::model::ErrorResponse>(response);
    }
    return executeGetAccountTransactionsWithPager(*qry);
  }
  if (instanceof <iroha::model::GetAccountAssetsTransactionsWithPager>(query.get())) {
    auto qry = std::static_pointer_cast<
      const iroha::model::GetAccountAssetsTransactionsWithPager>(query);
    if (!validate(*qry)) {
      iroha::model::ErrorResponse response;
      response.query_hash = iroha::hash(*qry);
      response.reason = model::ErrorResponse::STATEFUL_INVALID;
      return std::make_shared<iroha::model::ErrorResponse>(response);
    }
    return executeGetAccountAssetsTransactionsWithPager(*qry);
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

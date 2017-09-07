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
#include "model/queries/responses/account_assets_response.hpp"
#include "model/queries/responses/account_response.hpp"
#include "model/queries/responses/error_response.hpp"
#include "model/queries/responses/signatories_response.hpp"
#include "model/queries/responses/transactions_response.hpp"

iroha::model::QueryProcessingFactory::QueryProcessingFactory(
    std::shared_ptr<ametsuchi::WsvQuery> wsvQuery,
    std::shared_ptr<ametsuchi::BlockQuery> blockQuery)
    : _wsvQuery(wsvQuery), _blockQuery(blockQuery) {}

bool iroha::model::QueryProcessingFactory::validate(
    const model::GetAccount& query) {
  auto creator = _wsvQuery->getAccount(query.creator_account_id);
  // TODO: check signatures
  return
      // Creator account exits
      creator.has_value() &&
      // Creator has permission to read, or account = creator
      (creator.value().permissions.read_all_accounts ||
       query.account_id == query.creator_account_id);
}

bool iroha::model::QueryProcessingFactory::validate(
    const model::GetSignatories& query) {
  auto creator = _wsvQuery->getAccount(query.creator_account_id);
  return
      // Creator account exits
      creator.has_value() &&
      // Creator has permission to read, or account = creator
      (creator.value().permissions.read_all_accounts ||
       query.account_id == query.creator_account_id);
}

bool iroha::model::QueryProcessingFactory::validate(
    const model::GetAccountAssets& query) {
  auto creator = _wsvQuery->getAccount(query.creator_account_id);
  return
      // Creator account exits
      creator.has_value() &&
      // Creator has permission to read, or account = creator
      (creator.value().permissions.read_all_accounts ||
       query.account_id == query.creator_account_id);
}

bool iroha::model::QueryProcessingFactory::validate(
    const model::GetAccountTransactions& query) {
  auto creator = _wsvQuery->getAccount(query.creator_account_id);
  return
      // Creator account exits
      creator.has_value() &&
      // Creator has permission to read, or account = creator
      (creator.value().permissions.read_all_accounts ||
       query.account_id == query.creator_account_id);
}

bool iroha::model::QueryProcessingFactory::validate(
    const model::GetAccountAssetTransactions& query) {
  auto creator = _wsvQuery->getAccount(query.creator_account_id);
  return
      // Creator account exits
      creator.has_value() &&
      // Creator has permission to read, or account = creator
      (creator.value().permissions.read_all_accounts ||
       query.account_id == query.creator_account_id);
}

std::shared_ptr<iroha::model::QueryResponse>
iroha::model::QueryProcessingFactory::executeGetAccount(
    const model::GetAccount& query) {
  auto acc = _wsvQuery->getAccount(query.account_id);
  if (!acc.has_value()) {
    iroha::model::ErrorResponse response;
    response.query_hash = query.query_hash;
    response.reason = iroha::model::ErrorResponse::NO_ACCOUNT;
    return std::make_shared<ErrorResponse>(response);
  }
  iroha::model::AccountResponse response;
  response.account = acc.value();
  response.query_hash = query.query_hash;
  return std::make_shared<iroha::model::AccountResponse>(response);
}

std::shared_ptr<iroha::model::QueryResponse>
iroha::model::QueryProcessingFactory::executeGetAccountAssets(
    const model::GetAccountAssets& query) {
  auto acct_asset = _wsvQuery->getAccountAsset(query.account_id, query.asset_id);
  if (!acct_asset.has_value()) {
    iroha::model::ErrorResponse response;
    response.query_hash = query.query_hash;
    response.reason = iroha::model::ErrorResponse::NO_ACCOUNT_ASSETS;
    return std::make_shared<iroha::model::ErrorResponse>(response);
  }
  auto asset = _wsvQuery->getAsset(query.asset_id);
  // TODO: Add format with precision balance
  iroha::model::AccountAssetResponse response;
  response.acct_asset = acct_asset.value();
  response.query_hash = query.query_hash;
  return std::make_shared<iroha::model::AccountAssetResponse>(response);
}

std::shared_ptr<iroha::model::QueryResponse>
iroha::model::QueryProcessingFactory::executeGetAccountAssetTransactions(
    const model::GetAccountAssetTransactions& query) {
  auto acc_asset_tx = _blockQuery->getAccountAssetTransactions(query.account_id, query.asset_id);
  iroha::model::TransactionsResponse response;
  response.query_hash = query.query_hash;
  response.transactions = acc_asset_tx;
  return std::make_shared<iroha::model::TransactionsResponse>(response);
}

std::shared_ptr<iroha::model::QueryResponse>
iroha::model::QueryProcessingFactory::executeGetAccountTransactions(
    const model::GetAccountTransactions& query) {
  auto acc_tx = _blockQuery->getAccountTransactions(query.account_id);
  iroha::model::TransactionsResponse response;
  response.query_hash = query.query_hash;
  response.transactions = acc_tx;
  return std::make_shared<iroha::model::TransactionsResponse>(response);
}

std::shared_ptr<iroha::model::QueryResponse>
iroha::model::QueryProcessingFactory::executeGetSignatories(
    const model::GetSignatories& query) {
  auto signs = _wsvQuery->getSignatories(query.account_id);
  if (!signs.has_value()) {
    iroha::model::ErrorResponse response;
    response.query_hash = query.query_hash;
    response.reason = model::ErrorResponse::NO_SIGNATORIES;
    return std::make_shared<iroha::model::ErrorResponse>(response);
  }
  iroha::model::SignatoriesResponse response;
  response.query_hash = query.query_hash;
  response.keys = signs.value();
  return std::make_shared<iroha::model::SignatoriesResponse>(response);
}

std::shared_ptr<iroha::model::QueryResponse>
iroha::model::QueryProcessingFactory::execute(
    std::shared_ptr<const model::Query> query) {
  if (instanceof <iroha::model::GetAccount>(query.get())) {
    auto qry = std::static_pointer_cast<const iroha::model::GetAccount>(query);

    if (!validate(*qry)) {
      iroha::model::ErrorResponse response;
      response.query_hash = qry->query_hash;
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
      response.query_hash = qry->query_hash;
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
      response.query_hash = qry->query_hash;
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
      response.query_hash = qry->query_hash;
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
      response.query_hash = qry->query_hash;
      response.reason = model::ErrorResponse::STATEFUL_INVALID;
      return std::make_shared<iroha::model::ErrorResponse>(response);
    }
    return executeGetAccountAssetTransactions(*qry);
  }
  iroha::model::ErrorResponse response;
  response.query_hash = query->query_hash;
  response.reason = model::ErrorResponse::NOT_SUPPORTED;
  return std::make_shared<iroha::model::ErrorResponse>(response);
}

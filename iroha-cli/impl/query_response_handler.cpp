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

#include "query_response_handler.hpp"
#include "logger/logger.hpp"
#include "model/converters/pb_common.hpp"

using namespace iroha::protocol;
namespace iroha_cli {

  QueryResponseHandler::QueryResponseHandler()
      : log_(logger::log("QueryResponseHandler")) {
    handler_map_[QueryResponse::ResponseCase::kErrorResponse] =
        &QueryResponseHandler::handleErrorResponse;
    handler_map_[QueryResponse::ResponseCase::kAccountResponse] =
        &QueryResponseHandler::handleAccountResponse;
    handler_map_[QueryResponse::ResponseCase::kAccountAssetsResponse] =
        &QueryResponseHandler::handleAccountAssetsResponse;
    handler_map_[QueryResponse::ResponseCase::kSignatoriesResponse] =
        &QueryResponseHandler::handleSignatoriesResponse;
    handler_map_[QueryResponse::ResponseCase::kTransactionsResponse] =
        &QueryResponseHandler::handleTransactionsResponse;
    handler_map_[QueryResponse::ResponseCase::kRolesResponse] =
        &QueryResponseHandler::handleRolesResponse;
    handler_map_[QueryResponse::ResponseCase::kRolePermissionsResponse] =
        &QueryResponseHandler::handleRolePermissionsResponse;
    handler_map_[QueryResponse::ResponseCase::kAssetResponse] =
        &QueryResponseHandler::handleAssetResponse;

    // Error responses:
    error_handler_map_[ErrorResponse::STATEFUL_INVALID] =
        "Query is stateful invalid";
    error_handler_map_[ErrorResponse::STATELESS_INVALID] =
        "Query is stateless invalid";
    error_handler_map_[ErrorResponse::NO_ACCOUNT] = "Account not found";
    error_handler_map_[ErrorResponse::NO_ACCOUNT_ASSETS] =
        "Account assets not found";
    error_handler_map_[ErrorResponse::NO_SIGNATORIES] = "No signatories found";
    error_handler_map_[ErrorResponse::NOT_SUPPORTED] = "Query not supported";
    error_handler_map_[ErrorResponse::NO_ROLES] = "No roles in the system";
    error_handler_map_[ErrorResponse::NO_ASSET] = "No asset found";
  }

  void QueryResponseHandler::handle(
      const iroha::protocol::QueryResponse &response) {
    auto it = handler_map_.find(response.response_case());
    if (it != handler_map_.end()) {
      (this->*it->second)(response);
    } else {
      log_->error("Response Handle {} not Implemented",
                  response.response_case());
    }
  }

  enum PrefixId {
    kAccountId,
    kAssetId,
    kAmount,
    kDomainId,
    kSignatories,
    kPrecision,
    kRoles,
    kJsonData,
    kCreatorId,
    kDefault,
  };

  const std::map<PrefixId, const char *> prefix{
      {kAccountId, "-Account Id:- {}"},
      {kAssetId, "-Asset Id- {}"},
      {kAmount, "-Balance- {}"},
      {kDomainId, "-Domain- {}"},
      {kSignatories, "-Signatory- {}"},
      {kPrecision, "-Precision- {}"},
      {kRoles, "-Roles-: "},
      {kJsonData, "-Data-: {}"},
      {kCreatorId, "-Creator Id- {}"},
      {kDefault, " {} "}};

  void QueryResponseHandler::handleErrorResponse(
      const iroha::protocol::QueryResponse &response) {
    auto it = error_handler_map_.find((response.error_response().reason()));
    if (it != error_handler_map_.end()) {
      log_->error(it->second);
    } else {
      // Response of some other type received
      log_->error("Error Response Handle of type {} not Implemented",
                  response.error_response().reason());
    }
  }

  void QueryResponseHandler::handleAccountResponse(
      const iroha::protocol::QueryResponse &response) {
    auto account = response.account_response().account();
    log_->info("[Account]:");
    log_->info(prefix.at(kAccountId), account.account_id());
    log_->info(prefix.at(kDomainId), account.domain_id());

    log_->info(prefix.at(kRoles));
    auto roles = response.account_response().account_roles();
    std::for_each(roles.begin(), roles.end(), [this](auto role) {
      log_->info(prefix.at(kDefault), role);
    });
    log_->info(prefix.at(kJsonData), account.json_data());
  }

  void QueryResponseHandler::handleRolesResponse(
      const iroha::protocol::QueryResponse &response) {
    auto roles = response.roles_response().roles();
    std::for_each(roles.begin(), roles.end(), [this](auto role) {
      log_->info(prefix.at(kDefault), role);
    });
  }

  void QueryResponseHandler::handleRolePermissionsResponse(
      const iroha::protocol::QueryResponse &response) {
    auto perms = response.role_permissions_response().permissions();
    std::for_each(perms.begin(), perms.end(), [this](auto perm) {
      log_->info(prefix.at(kDefault), perm);
    });
  }

  void QueryResponseHandler::handleAccountAssetsResponse(
      const iroha::protocol::QueryResponse &response) {
    auto acc_assets = response.account_assets_response().account_asset();
    log_->info("[Account Assets]");
    log_->info(prefix.at(kAccountId), acc_assets.account_id());
    log_->info(prefix.at(kAssetId), acc_assets.asset_id());
    auto balance =
        iroha::model::converters::deserializeAmount(acc_assets.balance());
    log_->info(prefix.at(kAmount), balance.to_string());
  }

  void QueryResponseHandler::handleSignatoriesResponse(
      const iroha::protocol::QueryResponse &response) {
    auto signatories = response.signatories_response().keys();
    log_->info("[Signatories]");
    std::for_each(
        signatories.begin(), signatories.end(), [this](auto signatory) {
          log_->info(prefix.at(kSignatories), signatory);
        });
  }

  void QueryResponseHandler::handleAssetResponse(
      const iroha::protocol::QueryResponse &response) {
    auto asset = response.asset_response().asset();
    log_->info("[Asset]");
    log_->info(prefix.at(kAssetId), asset.asset_id());
    log_->info(prefix.at(kDomainId), asset.domain_id());
    log_->info(prefix.at(kPrecision), asset.precision());
  }

  void QueryResponseHandler::handleTransactionsResponse(
      const iroha::protocol::QueryResponse &response) {
    auto txs = response.transactions_response().transactions();
    std::for_each(txs.begin(), txs.end(), [this](auto tx) {
      log_->info("[Transaction]");
      log_->info(prefix.at(kCreatorId), tx.payload().creator_account_id());
      // TODO 13/09/17 grimadas: add other fields: tx head, tx body IR-507
    });
  }

}  // namespace iroha_cli

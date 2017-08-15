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
    error_handler_map_[ErrorResponse::WRONG_FORMAT] = "Query has wrong format";
  }

  bool QueryResponseHandler::handle(
      const iroha::protocol::QueryResponse &response) {
    auto it = handler_map_.find(response.response_case());
    if (it != handler_map_.end()) {
      return (this->*it->second)(response);
    } else {
      log_->error("Response Handle not Implemented");
      return false;
    }
  }

  bool QueryResponseHandler::handleErrorResponse(
      const iroha::protocol::QueryResponse &response) {
    auto it = error_handler_map_.find((response.error_response().reason()));
    if (it != error_handler_map_.end()) {
      log_->error(it->second);
      return false;
    } else {
      // Response of some other type received
      log_->error("Error Response Handle of type {} not Implemented",
                  response.error_response().reason());
      return false;
    }
  }

  bool QueryResponseHandler::handleAccountResponse(
      const iroha::protocol::QueryResponse &response) {
    auto account = response.account_response().account();
    log_->info("[Account]:");
    log_->info("-Id:- {}", account.account_id());
    // TODO : print permissions
    log_->info("-Domain- {}", account.domain_name());
    return true;
  }

  bool QueryResponseHandler::handleAccountAssetsResponse(
      const iroha::protocol::QueryResponse &response) {
    auto acc_assets = response.account_assets_response().account_asset();
    log_->info("[Account Assets]");
    log_->info("-Account Id- {}", acc_assets.account_id());
    log_->info("-Asset Id- {}", acc_assets.asset_id());
    log_->info("-Balance- {}", acc_assets.balance());
    return true;
  }

  bool QueryResponseHandler::handleSignatoriesResponse(
      const iroha::protocol::QueryResponse &response) {
    auto signatories = response.signatories_response().keys();
    log_->info("[Signatories]");
    std::for_each(
        signatories.begin(), signatories.end(),
        [this](auto signatory) { log_->info("-Signatory- {}", signatory); });
    return true;
  }

  bool QueryResponseHandler::handleTransactionsResponse(
      const iroha::protocol::QueryResponse &response) {
    auto txs = response.transactions_response().transactions();
    log_->info("[Transactions]");
    std::for_each(txs.begin(), txs.end(), [this](auto tx) {
      log_->info("-[tx]-");
      log_->info("--[Creator Id] -- {}", tx.meta().creator_account_id());
      // TODO: add other fields
    });
    return true;
  }

}  // namespace iroha_cli

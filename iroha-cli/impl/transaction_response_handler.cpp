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
#include "transaction_response_handler.hpp"
#include "logger/logger.hpp"

namespace iroha_cli {

  void TransactionResponseHandler::handle(
      const CliClient::TxStatus status) const {
    switch (status) {
      case iroha_cli::CliClient::OK:
        log_->info("Transaction successfully sent");
        break;
        /*
      case iroha_cli::CliClient::NOT_VALID:
        log_->error("Transaction is not valid");
        break;
         */
    }
  }
  TransactionResponseHandler::TransactionResponseHandler()
      : log_(logger::log("TransactionResponseHandler")) {}

}  // namespace iroha_cli

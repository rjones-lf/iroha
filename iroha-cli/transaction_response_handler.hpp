/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_CLI_TRANSACTION_RESPONSE_HANDLER_HPP
#define IROHA_CLI_TRANSACTION_RESPONSE_HANDLER_HPP

#include "client.hpp"  // for CliClient::TxStatus (yuck!)

namespace spdlog {
  class logger;
}

namespace iroha_cli {

  class TransactionResponseHandler {
   public:
    explicit TransactionResponseHandler(
        std::shared_ptr<spdlog::logger> log =
            logger::log("TransactionResponseHandler"));
    /**
     * Handle response from Iroha
     * @param status of transaction
     */
    void handle(const CliClient::TxStatus status) const;

   private:
    std::shared_ptr<spdlog::logger> log_;
  };
}  // namespace iroha_cli

#endif  // IROHA_CLI_TRANSACTION_RESPONSE_HANDLER_HPP

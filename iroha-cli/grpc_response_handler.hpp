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

#ifndef IROHA_CLI_GRPC_RESPONSE_HANDLER_HPP
#define IROHA_CLI_GRPC_RESPONSE_HANDLER_HPP

#include "query_response_handler.hpp"
#include "transaction_response_handler.hpp"

namespace spdlog {
  class logger;
}

namespace iroha_cli {
  class GrpcResponseHandler {
   public:
    GrpcResponseHandler();
    /**
     * Handle iroha GRPC TxResponse
     * @param response
     */
    void handle(CliClient::Response<CliClient::TxStatus> response);
    /**
     * Handle Iroha GRPC QueryResponse
     * @param response
     */
    void handle(CliClient::Response<iroha::protocol::QueryResponse> response);

   private:
    TransactionResponseHandler tx_handler_;
    QueryResponseHandler query_handler_;
    void handleGrpcErrors(grpc::StatusCode code);
    std::shared_ptr<spdlog::logger> log_;
    std::unordered_map<int, std::string> handler_map_;
  };
}  // namespace iroha_cli

#endif  // IROHA_GRPC_RESPONSE_HANDLER_HPP

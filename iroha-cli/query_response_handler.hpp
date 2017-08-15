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

#ifndef IROHA_CLI_QUERY_RESPONSE_HANDLER_HPP
#define IROHA_CLI_QUERY_RESPONSE_HANDLER_HPP
#include <typeindex>
#include <unordered_map>
#include "logger/logger.hpp"
#include "responses.pb.h"
#include <memory>
#include <map>

namespace iroha_cli {
  class QueryResponseHandler {
   public:
    QueryResponseHandler();

    /**
     * Handle query response
     * @param response - iroha protocol
     */
    bool handle(const iroha::protocol::QueryResponse& response);

   private:
    bool handleErrorResponse(const iroha::protocol::QueryResponse& response);
    bool handleAccountResponse(const iroha::protocol::QueryResponse& response);
    bool handleAccountAssetsResponse(
        const iroha::protocol::QueryResponse& response);
    bool handleTransactionsResponse(
        const iroha::protocol::QueryResponse& response);
    bool handleSignatoriesResponse(
        const iroha::protocol::QueryResponse& response);
    // -- --
    using Handler =
        bool (QueryResponseHandler::*)(const iroha::protocol::QueryResponse&);
    std::unordered_map<int , Handler> handler_map_;
    std::unordered_map<int, std::string> error_handler_map_;

    std::shared_ptr<spdlog::logger> log_;
  };

}  // namespace iroha_cli
#endif  // IROHA_QUERY_RESPONSE_HANDLER_HPP

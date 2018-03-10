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

#include <map>
#include <memory>
#include <typeindex>
#include <unordered_map>

#include "responses.pb.h"

namespace spdlog {
  class logger;
}

namespace iroha_cli {
  /*
  workaround for circle-ci compilation issue; see
  http://www.open-std.org/jtc1/sc22/wg21/docs/lwg-defects.html#2148 and
  https://stackoverflow.com/questions/18837857/cant-use-enum-class-as-unordered-map-key
  for more details
  */
  struct EnumTypeHash {
    template <typename T>
    std::size_t operator()(T t) const {
      return static_cast<std::size_t>(t);
    }
  };

  class QueryResponseHandler {
   public:
    QueryResponseHandler();

    /**
     * Handle query response
     * @param response - iroha protocol object
     */
    void handle(const iroha::protocol::QueryResponse &response);

   private:
    void handleErrorResponse(const iroha::protocol::QueryResponse &response);
    void handleAccountResponse(const iroha::protocol::QueryResponse &response);
    void handleAccountAssetsResponse(
        const iroha::protocol::QueryResponse &response);
    void handleTransactionsResponse(
        const iroha::protocol::QueryResponse &response);
    void handleSignatoriesResponse(
        const iroha::protocol::QueryResponse &response);
    void handleRolesResponse(const iroha::protocol::QueryResponse &response);
    void handleRolePermissionsResponse(
        const iroha::protocol::QueryResponse &response);
    void handleAssetResponse(const iroha::protocol::QueryResponse &response);
    // -- --
    using Handler =
        void (QueryResponseHandler::*)(const iroha::protocol::QueryResponse &);
    using QueryResponseCode = iroha::protocol::QueryResponse::ResponseCase;
    using ErrorResponseCode = iroha::protocol::ErrorResponse::Reason;

    // Map  QueryResponse code -> Handle Method
    std::unordered_map<QueryResponseCode, Handler, EnumTypeHash> handler_map_;
    // Map ErrorResponse code -> String to print
    std::unordered_map<ErrorResponseCode, std::string, EnumTypeHash>
        error_handler_map_;

    std::shared_ptr<spdlog::logger> log_;
  };

}  // namespace iroha_cli
#endif  // IROHA_QUERY_RESPONSE_HANDLER_HPP

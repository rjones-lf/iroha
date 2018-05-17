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

#ifndef IROHACLI_CLIENT_HPP
#define IROHACLI_CLIENT_HPP

#include <string>

#include "torii/command_client.hpp"
#include "torii/query_client.hpp"

namespace shared_model {
  namespace interface {
    class Transaction;
    class Query;
  }  // namespace interface
}  // namespace shared_model

namespace iroha_cli {

  class CliClient {
   public:
    template <typename T>
    struct Response {
      grpc::Status status;
      T answer;
    };

    // TODO 13/09/17 luckychess: check if we need more status codes IR-494
    enum TxStatus { OK };

    CliClient(std::string target_ip, int port);
    /**
     * Send Transaction to Iroha Peer, i.e. target_ip:port
     * @param tx
     * @return
     */
    CliClient::Response<CliClient::TxStatus> sendTx(
        const shared_model::interface::Transaction &tx);

    /**
     * Send Query to Iroha Peer, i.e. target_ip:port
     * @param query
     * @return
     */
    CliClient::Response<iroha::protocol::QueryResponse> sendQuery(
        const shared_model::interface::Query &query);

    CliClient::Response<iroha::protocol::ToriiResponse> getTxStatus(
        std::string tx_hash);

   private:
    torii::CommandSyncClient command_client_;
    torii_utils::QuerySyncClient query_client_;
  };
}  // namespace iroha_cli

#endif  // IROHACLI_CLIENT_CPP_HPP

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

#ifndef IROHA_CLIENT_HPP
#define IROHA_CLIENT_HPP

#include <string>
#include "torii_utils/query_client.hpp"
#include "torii/command_client.hpp"

namespace iroha_cli {

  class CliClient {
   public:
    template <typename T>
    struct Response{
      grpc::Status status;
      T answer;
    };

    enum TxStatus { WRONG_FORMAT, OK,  };

    CliClient(std::string target_ip, int port);
    /**
     * Send Transaction to Iroha Peer, i.e. target_ip:port
     * @param tx
     * @return
     */
    CliClient::Response<CliClient::TxStatus> sendTx(iroha::model::Transaction tx);

    /**
     * Send Query to Iroha Peer, i.e. target_ip:port
     * @param query
     * @return
     */
    CliClient::Response<iroha::protocol::QueryResponse> sendQuery(
        std::shared_ptr<iroha::model::Query> query);


   private:
    torii::CommandSyncClient command_client_;
    torii_utils::QuerySyncClient query_client_;
  };
}  // namespace iroha_cli

#endif  // IROHA_CLIENT_CPP_HPP

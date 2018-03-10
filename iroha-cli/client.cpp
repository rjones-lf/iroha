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

#include "client.hpp"
#include "model/converters/json_query_factory.hpp"
#include "model/converters/json_transaction_factory.hpp"
#include "model/converters/pb_query_factory.hpp"
#include "model/converters/pb_transaction_factory.hpp"

namespace iroha_cli {

  CliClient::CliClient(std::string target_ip, int port)
      : command_client_(target_ip, port), query_client_(target_ip, port) {}

  CliClient::Response<CliClient::TxStatus> CliClient::sendTx(
      iroha::model::Transaction tx) {
    CliClient::Response<CliClient::TxStatus> response;
    // Convert to protobuf
    iroha::model::converters::PbTransactionFactory factory;
    auto pb_tx = factory.serialize(tx);
    // Send to iroha:
    response.status = command_client_.Torii(pb_tx);

    // TODO 12/10/2017 neewy implement return of real transaction status IR-494
    response.answer = TxStatus::OK;

    return response;
  }

  CliClient::Response<iroha::protocol::ToriiResponse> CliClient::getTxStatus(
      std::string tx_hash) {
    CliClient::Response<iroha::protocol::ToriiResponse> response;
    // Send to iroha:
    iroha::protocol::TxStatusRequest statusRequest;
    statusRequest.set_tx_hash(tx_hash);
    iroha::protocol::ToriiResponse toriiResponse;
    response.status = command_client_.Status(statusRequest, toriiResponse);
    response.answer = toriiResponse;

    return response;
  }

  CliClient::Response<iroha::protocol::QueryResponse> CliClient::sendQuery(
      std::shared_ptr<iroha::model::Query> query) {
    CliClient::Response<iroha::protocol::QueryResponse> response;
    // Convert to proto and send to Iroha
    iroha::model::converters::PbQueryFactory pb_factory;
    auto pb_query = pb_factory.serialize(query);
    iroha::protocol::QueryResponse query_response;
    response.status = query_client_.Find(pb_query.value(), query_response);
    response.answer = query_response;
    return response;
  }

}  // namespace iroha_cli

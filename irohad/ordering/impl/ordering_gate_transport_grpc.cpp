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

#include "ordering_gate_transport_grpc.hpp"

using namespace iroha::ordering;
using namespace iroha;

void OrderingGateTransportGrpc::propagate(std::shared_ptr<const model::Transaction> transaction)  {
  auto call = new AsyncClientCall;

  call->response_reader = client_->AsyncSendTransaction(
          &call->context, factory_.serialize(*transaction), &cq_);

  call->response_reader->Finish(&call->reply, &call->status, call);
}

OrderingGateTransportGrpc::OrderingGateTransportGrpc(const std::string &server_address):
        client_(proto::OrderingService::NewStub(grpc::CreateChannel(
                server_address, grpc::InsecureChannelCredentials()))) {
}

void OrderingGateTransportGrpc::subscribe(
        std::shared_ptr<iroha::network::OrderingGateNotification> subscriber) {
  subscriber_ = subscriber;
}


std::shared_ptr<model::Proposal> OrderingGateTransportGrpc::getProposal(const iroha::ordering::proto::Proposal *request) {
  auto transactions =
          decltype(std::declval<model::Proposal>().transactions)();
  for (const auto &tx : request->transactions()) {
    transactions.push_back(*factory_.deserialize(tx));
  }

  auto proposal = std::make_shared<model::Proposal>(transactions);

  return proposal;
};

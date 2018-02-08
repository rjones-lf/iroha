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
using namespace iroha::protocol;

grpc::Status OrderingGateTransportGrpc::onProposal(
    ::grpc::ServerContext *context,
    const protocol::Proposal *request,
    ::google::protobuf::Empty *response) {
  log_->info("receive proposal");

  auto transactions = decltype(std::declval<model::Proposal>().transactions)();
  for (const auto &tx : request->transactions()) {
    transactions.push_back(*factory_.deserialize(tx));
  }
  log_->info("transactions in proposal: {}", transactions.size());

  model::Proposal proposal(transactions);
  proposal.height = request->height();
  proposal.created_time = request->created_time();
  if (not subscriber_.expired()) {
    subscriber_.lock()->onProposal(std::move(proposal));
  } else {
    log_->error("(onProposal) No subscriber");
  }

  return grpc::Status::OK;
}

OrderingGateTransportGrpc::OrderingGateTransportGrpc(
    const std::string &server_address)
    : client_(proto::OrderingServiceTransportGrpc::NewStub(grpc::CreateChannel(
          server_address, grpc::InsecureChannelCredentials()))),
      log_(logger::log("OrderingGate")) {}

void OrderingGateTransportGrpc::propagate_transaction(
    std::shared_ptr<const model::Transaction> transaction) {
  log_->info("Propagate tx (on transport)");
  auto call = new AsyncClientCall;

  call->response_reader = client_->AsynconTransaction(
      &call->context, factory_.serialize(*transaction), &cq_);

  call->response_reader->Finish(&call->reply, &call->status, call);
}

void OrderingGateTransportGrpc::subscribe(
    std::shared_ptr<iroha::network::OrderingGateNotification> subscriber) {
  log_->info("Subscribe");
  subscriber_ = subscriber;
}

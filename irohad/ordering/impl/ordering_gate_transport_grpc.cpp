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

#include "backend/protobuf/transaction.hpp"
#include "builders/protobuf/proposal.hpp"
#include "interfaces/common_objects/types.hpp"

using namespace iroha::ordering;

grpc::Status OrderingGateTransportGrpc::onProposal(
    ::grpc::ServerContext *context,
    const iroha::protocol::Proposal *request,
    ::google::protobuf::Empty *response) {
  log_->info("receive proposal");

  std::vector<shared_model::proto::Transaction> transactions;
  for (const auto &tx : request->transactions()) {
    transactions.emplace_back(tx);
  }
  log_->info("transactions in proposal: {}", transactions.size());

  auto proposal = std::make_shared<shared_model::proto::Proposal>(
      shared_model::proto::ProposalBuilder()
          .transactions(transactions)
          .height(request->height())
          .createdTime(request->created_time())
          .build());

  if (not subscriber_.expired()) {
    subscriber_.lock()->onProposal(std::move(proposal));
  } else {
    log_->error("(onProposal) No subscriber");
  }

  return grpc::Status::OK;
}

OrderingGateTransportGrpc::OrderingGateTransportGrpc(
    const std::string &server_address)
    : network::AsyncGrpcClient<google::protobuf::Empty>(
          logger::log("OrderingGate")),
      client_(proto::OrderingServiceTransportGrpc::NewStub(grpc::CreateChannel(
          server_address, grpc::InsecureChannelCredentials()))) {}

void OrderingGateTransportGrpc::propagateTransaction(
    std::shared_ptr<const shared_model::interface::Transaction> transaction) {
  log_->info("Propagate tx (on transport)");
  auto call = new AsyncClientCall;

  auto transaction_transport =
      static_cast<const shared_model::proto::Transaction &>(*transaction)
          .getTransport();
  log_->debug("Propagating: '{}'", transaction_transport.DebugString());
  call->response_reader =
      client_->AsynconTransaction(&call->context, transaction_transport, &cq_);

  call->response_reader->Finish(&call->reply, &call->status, call);
}

void OrderingGateTransportGrpc::subscribe(
    std::shared_ptr<iroha::network::OrderingGateNotification> subscriber) {
  log_->info("Subscribe");
  subscriber_ = subscriber;
}

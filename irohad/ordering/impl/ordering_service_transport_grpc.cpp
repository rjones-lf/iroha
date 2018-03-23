/**
 * Copyright Soramitsu Co., Ltd. 2018 All Rights Reserved.
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
#include "ordering/impl/ordering_service_transport_grpc.hpp"

#include "backend/protobuf/transaction.hpp"
#include "builders/protobuf/proposal.hpp"

using namespace iroha::ordering;

void OrderingServiceTransportGrpc::subscribe(
    std::shared_ptr<iroha::network::OrderingServiceNotification> subscriber) {
  subscriber_ = subscriber;
}

grpc::Status OrderingServiceTransportGrpc::onTransaction(
    ::grpc::ServerContext *context,
    const iroha::protocol::Transaction *request,
    ::google::protobuf::Empty *response) {
  if (subscriber_.expired()) {
    log_->error("No subscriber");
  } else {
    subscriber_.lock()->onTransaction(
        std::make_shared<shared_model::proto::Transaction>(
            iroha::protocol::Transaction(*request)));
  }

  return ::grpc::Status::OK;
}

void OrderingServiceTransportGrpc::publishProposal(
    std::unique_ptr<shared_model::interface::Proposal> proposal,
    const std::vector<std::string> &peers) {
  std::unordered_map<std::string,
                     std::unique_ptr<proto::OrderingGateTransportGrpc::Stub>>
      peers_map;

  for (const auto &peer : peers) {
    peers_map[peer] = proto::OrderingGateTransportGrpc::NewStub(
        grpc::CreateChannel(peer, grpc::InsecureChannelCredentials()));
  }

  for (const auto &peer : peers_map) {
    auto call = new AsyncClientCall;

    auto proto = static_cast<shared_model::proto::Proposal *>(proposal.get());
    call->response_reader = peer.second->AsynconProposal(
        &call->context, proto->getTransport(), &cq_);

    call->response_reader->Finish(&call->reply, &call->status, call);
  }
}

OrderingServiceTransportGrpc::OrderingServiceTransportGrpc()
    : log_(logger::testLog("OrderingServiceTransportGrpc")) {}

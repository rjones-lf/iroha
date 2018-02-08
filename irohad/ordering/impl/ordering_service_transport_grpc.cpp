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
#include "ordering/impl/ordering_service_transport_grpc.hpp"
#include "datetime/time.hpp"

using namespace iroha::ordering;
using namespace iroha::protocol;
using namespace iroha::model;
using namespace iroha::network;

void OrderingServiceTransportGrpc::subscribe(
    std::shared_ptr<OrderingServiceNotification> subscriber) {
  subscriber_ = subscriber;
}

grpc::Status OrderingServiceTransportGrpc::onTransaction(
    ::grpc::ServerContext *context,
    const protocol::Transaction *request,
    ::google::protobuf::Empty *response) {
  if (subscriber_.expired()) {
    log_->error("No subscriber");
  } else {
    subscriber_.lock()->onTransaction(*factory_.deserialize(*request));
  }

  return ::grpc::Status::OK;
}

void OrderingServiceTransportGrpc::publishProposal(
    model::Proposal &&proposal, const std::vector<std::string> &peers) {
  std::unordered_map<std::string,
                     std::unique_ptr<proto::OrderingGateTransportGrpc::Stub>>
      peers_map;

  for (const auto &peer : peers) {
    peers_map[peer] = proto::OrderingGateTransportGrpc::NewStub(
        grpc::CreateChannel(peer, grpc::InsecureChannelCredentials()));
  }

  protocol::Proposal pb_proposal;
  pb_proposal.set_height(proposal.height);
  pb_proposal.set_created_time(iroha::time::now());
  for (const auto &tx : proposal.transactions) {
    new (pb_proposal.add_transactions())
        protocol::Transaction(factory_.serialize(tx));
  }

  for (const auto &peer : peers_map) {
    auto call = new AsyncClientCall;

    call->response_reader =
        peer.second->AsynconProposal(&call->context, pb_proposal, &cq_);

    call->response_reader->Finish(&call->reply, &call->status, call);
  }
}

OrderingServiceTransportGrpc::OrderingServiceTransportGrpc()
    : log_(logger::testLog("OrderingServiceTransportGrpc")) {}

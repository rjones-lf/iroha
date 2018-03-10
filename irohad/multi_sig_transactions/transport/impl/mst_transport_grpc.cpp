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

#include "multi_sig_transactions/transport/mst_transport_grpc.hpp"

using namespace iroha::network;

MstTransportGrpc::MstTransportGrpc() : log_(logger::log("MstTransport")) {}

grpc::Status MstTransportGrpc::SendState(
    ::grpc::ServerContext* context,
    const ::iroha::network::transport::MstState* request,
    ::google::protobuf::Empty* response) {
  log_->info("MstState Received");

  MstState newState = MstState::empty();
  for (const auto& tx : request->transactions()) {
    // TODO: use monad after deserialize() will return optional
    newState += factory_.deserialize(tx);
  }
  log_->info("transactions in MstState: {}", newState.getTransactions().size());

  model::Peer from(request->peer().address(),
                   blob_t<32>::from_string(request->peer().pubkey()));
  subscriber_.lock()->onNewState(std::move(from), std::move(newState));

  return grpc::Status::OK;
}

void MstTransportGrpc::subscribe(
    std::shared_ptr<MstTransportNotification> notification) {
  subscriber_ = notification;
}

void MstTransportGrpc::sendState(ConstRefPeer to, ConstRefState providing_state) {
  log_->info("Propagate MstState to peer {}", to.address);
  auto client = transport::MstTransportGrpc::NewStub(
      grpc::CreateChannel(to.address, grpc::InsecureChannelCredentials()));

  AsyncClientCall* call = new AsyncClientCall;

  transport::MstState protoState;
  auto peer = protoState.mutable_peer();
  peer->set_pubkey(to.pubkey.to_string());
  peer->set_address(to.address);
  for (auto& tx : providing_state.getTransactions()) {
    auto addtxs = protoState.add_transactions();
    new (addtxs) protocol::Transaction(factory_.serialize(*tx));
  }

  call->response_reader =
      client->AsyncSendState(&call->context, protoState, &cq_);
  call->response_reader->Finish(&call->reply, &call->status, call);
}

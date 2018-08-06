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
#include "backend/protobuf/transaction.hpp"
#include "builders/protobuf/common_objects/proto_peer_builder.hpp"
#include "builders/protobuf/transport_builder.hpp"
#include "validators/default_validator.hpp"

using namespace iroha::network;

MstTransportGrpc::MstTransportGrpc(
    std::shared_ptr<network::AsyncGrpcClient<google::protobuf::Empty>>
        async_call)
    : async_call_(async_call) {}

grpc::Status MstTransportGrpc::SendState(
    ::grpc::ServerContext *context,
    const ::iroha::network::transport::MstState *request,
    ::google::protobuf::Empty *response) {
  async_call_->log_->info("MstState Received");

  MstState newState = MstState::empty();
  shared_model::proto::TransportBuilder<
      shared_model::proto::Transaction,
      shared_model::validation::DefaultUnsignedTransactionValidator>
      builder;
  for (const auto &tx : request->transactions()) {
    // TODO: use monad after deserialize() will return optional
    builder.build(tx).match(
        [&](iroha::expected::Value<shared_model::proto::Transaction> &v) {
          newState += std::make_shared<shared_model::proto::Transaction>(
              std::move(v.value));
        },
        [&](iroha::expected::Error<std::string> &e) {
          async_call_->log_->warn("Can't deserialize tx: {}", e.error);
        });
  }
  async_call_->log_->info("transactions in MstState: {}",
                          newState.getTransactions().size());

  auto &peer = request->peer();
  auto from = std::make_shared<shared_model::proto::Peer>(
      shared_model::proto::PeerBuilder()
          .address(peer.address())
          .pubkey(shared_model::crypto::PublicKey(peer.peer_key()))
          .build());
  subscriber_.lock()->onNewState(std::move(from), std::move(newState));

  return grpc::Status::OK;
}

void MstTransportGrpc::subscribe(
    std::shared_ptr<MstTransportNotification> notification) {
  subscriber_ = notification;
}

void MstTransportGrpc::sendState(const shared_model::interface::Peer &to,
                                 ConstRefState providing_state) {
  async_call_->log_->info("Propagate MstState to peer {}", to.address());
  std::unique_ptr<transport::MstTransportGrpc::StubInterface> client =
      transport::MstTransportGrpc::NewStub(grpc::CreateChannel(
          to.address(), grpc::InsecureChannelCredentials()));

  transport::MstState protoState;
  auto peer = protoState.mutable_peer();
  peer->set_peer_key(shared_model::crypto::toBinaryString(to.pubkey()));
  peer->set_address(to.address());
  for (auto &tx : providing_state.getTransactions()) {
    auto addtxs = protoState.add_transactions();
    // TODO (@l4l) 04/03/18 simplify with IR-1040
    new (addtxs) protocol::Transaction(
        std::static_pointer_cast<shared_model::proto::Transaction>(tx)
            ->getTransport());
  }

  async_call_->Call([&](auto context, auto cq) {
    return client->AsyncSendState(context, protoState, cq);
  });
}

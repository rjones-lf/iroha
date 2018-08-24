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
#include "interfaces/iroha_internal/transaction_sequence.hpp"
#include "validators/default_validator.hpp"
#include "validators/transactions_collection/batch_order_validator.hpp"

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

  shared_model::proto::TransportBuilder<
      shared_model::proto::Transaction,
      shared_model::validation::DefaultUnsignedTransactionValidator>
      builder;

  shared_model::interface::types::SharedTxsCollectionType collection;

  for (const auto &proto_tx : request->transactions()) {
    builder.build(proto_tx).match(
        [&](iroha::expected::Value<shared_model::proto::Transaction> &v) {
          collection.push_back(
              std::make_shared<shared_model::proto::Transaction>(
                  std::move(v.value)));

        },
        [&](iroha::expected::Error<std::string> &e) {
          async_call_->log_->warn("Can't deserialize tx: {}", e.error);
        });
  }

  using namespace shared_model::validation;
  auto new_state =
      shared_model::interface::TransactionSequence::createTransactionSequence(
          collection, DefaultSignedTransactionsValidator())
          .match(
              [](expected::Value<shared_model::interface::TransactionSequence>
                     &seq) {
                MstState new_state = MstState::empty();
                std::for_each(
                    seq.value.batches().begin(),
                    seq.value.batches().end(),
                    [&new_state](const auto &batch) {
                      new_state += std::make_shared<
                          shared_model::interface::TransactionBatch>(batch);
                    });
                return new_state;
              },
              [this](const auto &err) {
                async_call_->log_->warn("Can't create sequence: {}", err.error);
                return MstState::empty();
              });

  async_call_->log_->info("batches in MstState: {}",
                          new_state.getBatches().size());

  auto &peer = request->peer();
  auto from = std::make_shared<shared_model::proto::Peer>(
      shared_model::proto::PeerBuilder()
          .address(peer.address())
          .pubkey(shared_model::crypto::PublicKey(peer.peer_key()))
          .build());
  subscriber_.lock()->onNewState(std::move(from), std::move(new_state));

  return grpc::Status::OK;
}

void MstTransportGrpc::subscribe(
    std::shared_ptr<MstTransportNotification> notification) {
  subscriber_ = notification;
}

void MstTransportGrpc::sendState(const shared_model::interface::Peer &to,
                                 ConstRefState providing_state) {
  async_call_->log_->info("Propagate MstState to peer {}", to.address());
  auto client = transport::MstTransportGrpc::NewStub(
      grpc::CreateChannel(to.address(), grpc::InsecureChannelCredentials()));

  transport::MstState protoState;
  auto peer = protoState.mutable_peer();
  peer->set_peer_key(shared_model::crypto::toBinaryString(to.pubkey()));
  peer->set_address(to.address());
  for (auto &batch : providing_state.getBatches()) {
    for (auto &tx : batch->transactions()) {
      // TODO (@l4l) 04/03/18 simplify with IR-1040
      *protoState.add_transactions() =
          std::static_pointer_cast<shared_model::proto::Transaction>(tx)
              ->getTransport();
    }
  }

  async_call_->Call([&](auto context, auto cq) {
    return client->AsyncSendState(context, protoState, cq);
  });
}

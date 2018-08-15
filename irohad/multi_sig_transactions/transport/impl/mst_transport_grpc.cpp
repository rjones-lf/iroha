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

MstTransportGrpc::MstTransportGrpc()
    : AsyncGrpcClient<google::protobuf::Empty>(logger::log("MstTransport")) {}

grpc::Status MstTransportGrpc::SendState(
    ::grpc::ServerContext *context,
    const ::iroha::network::transport::MstState *request,
    ::google::protobuf::Empty *response) {
  log_->info("MstState Received");

  MstState newState = MstState::empty();
  shared_model::proto::TransportBuilder<
      shared_model::proto::Transaction,
      shared_model::validation::DefaultTransactionValidator>
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
          log_->warn("Can't deserialize tx: {}", e.error);
        });
  }

  using namespace shared_model::validation;
  shared_model::interface::TransactionSequence::createTransactionSequence(
      collection,
      UnsignedTransactionsCollectionValidator<DefaultTransactionValidator,
                                              BatchOrderValidator>())
      .match(
          [&newState](
              expected::Value<shared_model::interface::TransactionSequence>
                  &seq) {
            std::for_each(
                seq.value.batches().begin(),
                seq.value.batches().end(),
                [&newState](const auto &batch) {
                  newState += std::make_shared<
                      shared_model::interface::TransactionBatch>(batch);
                });
          },
          [this](const auto &err) {
            log_->warn("Can't create sequence: {}", err.error);
          });

  log_->info("batches in MstState: {}", newState.getBatches().size());

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
  log_->info("Propagate MstState to peer {}", to.address());
  auto client = transport::MstTransportGrpc::NewStub(
      grpc::CreateChannel(to.address(), grpc::InsecureChannelCredentials()));

  auto call = new AsyncClientCall;

  transport::MstState protoState;
  auto peer = protoState.mutable_peer();
  peer->set_peer_key(shared_model::crypto::toBinaryString(to.pubkey()));
  peer->set_address(to.address());
  for (auto &batch : providing_state.getBatches()) {
    for (auto &tx : batch->transactions()) {
      auto addtxs = protoState.add_transactions();
      // TODO (@l4l) 04/03/18 simplify with IR-1040
      new (addtxs) protocol::Transaction(
          std::static_pointer_cast<shared_model::proto::Transaction>(tx)
              ->getTransport());
    }
  }

  call->response_reader =
      client->AsyncSendState(&call->context, protoState, &cq_);
  call->response_reader->Finish(&call->reply, &call->status, call);
}

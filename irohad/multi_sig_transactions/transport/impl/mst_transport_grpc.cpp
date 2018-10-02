/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "multi_sig_transactions/transport/mst_transport_grpc.hpp"

#include "backend/protobuf/transaction.hpp"
#include "builders/protobuf/transport_builder.hpp"
#include "interfaces/iroha_internal/transaction_batch.hpp"
#include "interfaces/iroha_internal/transaction_sequence_factory.hpp"
#include "validators/default_validator.hpp"
#include "validators/transactions_collection/batch_order_validator.hpp"

using namespace iroha::network;

MstTransportGrpc::MstTransportGrpc(
    std::shared_ptr<network::AsyncGrpcClient<google::protobuf::Empty>>
        async_call,
    std::shared_ptr<shared_model::interface::CommonObjectsFactory> factory)
    : async_call_(std::move(async_call)), factory_(std::move(factory)) {}

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
      shared_model::interface::TransactionSequenceFactory::
          createTransactionSequence(collection,
                                    DefaultSignedTransactionsValidator())
              .match(
                  [](expected::Value<
                      shared_model::interface::TransactionSequence> &seq) {
                    MstState new_state = MstState::empty();
                    std::for_each(seq.value.batches().begin(),
                                  seq.value.batches().end(),
                                  [&new_state](const auto &batch) {
                                    new_state += batch;
                                  });
                    return new_state;
                  },
                  [this](const auto &err) {
                    async_call_->log_->warn("Can't create sequence: {}",
                                            err.error);
                    return MstState::empty();
                  });

  async_call_->log_->info("batches in MstState: {}",
                          new_state.getBatches().size());

  auto &peer = request->peer();
  factory_
      ->createPeer(peer.address(),
                   shared_model::crypto::PublicKey(peer.peer_key()))
      .match(
          [&](expected::Value<std::unique_ptr<shared_model::interface::Peer>>
                  &v) {
            subscriber_.lock()->onNewState(std::move(v.value),
                                           std::move(new_state));
          },
          [&](expected::Error<std::string> &e) {
            async_call_->log_->info(e.error);
          });

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
  for (auto &batch : providing_state.getBatches()) {
    for (auto &tx : batch->transactions()) {
      // TODO (@l4l) 04/03/18 simplify with IR-1040
      *protoState.add_transactions() =
          std::static_pointer_cast<shared_model::proto::Transaction>(tx)
              ->getTransport();
    }
  }

  // TODO: 15.09.2018 @x3medima17: IR-1709 replace synchronous SendState with
  // AsycnSendState,
  ::grpc::ClientContext context;
  ::google::protobuf::Empty empty;
  client->SendState(&context, protoState, &empty);

  //  async_call_->Call([&](auto context, auto cq) {
  //    return client->AsyncSendState(context, protoState, cq);
  //  });
}

/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "common/default_constructible_unary_fn.hpp"  // non-copyable value workaround

#include "multi_sig_transactions/transport/mst_transport_grpc.hpp"

#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include "backend/protobuf/transaction.hpp"
#include "cryptography/public_key.hpp"
#include "interfaces/iroha_internal/transaction_batch.hpp"
#include "interfaces/transaction.hpp"

using namespace iroha::network;

MstTransportGrpc::MstTransportGrpc(
    std::shared_ptr<network::AsyncGrpcClient<google::protobuf::Empty>>
        async_call,
    std::shared_ptr<shared_model::interface::CommonObjectsFactory> factory,
    std::shared_ptr<TransportFactoryType> transaction_factory,
    std::shared_ptr<shared_model::interface::TransactionBatchParser>
        batch_parser,
    std::shared_ptr<shared_model::interface::TransactionBatchFactory>
        transaction_batch_factory)
    : async_call_(std::move(async_call)),
      factory_(std::move(factory)),
      transaction_factory_(std::move(transaction_factory)),
      batch_parser_(std::move(batch_parser)),
      batch_factory_(std::move(transaction_batch_factory)) {}

shared_model::interface::types::SharedTxsCollectionType
MstTransportGrpc::deserializeTransactions(const transport::MstState *request) {
  return boost::copy_range<
      shared_model::interface::types::SharedTxsCollectionType>(
      request->transactions()
      | boost::adaptors::transformed(
            [&](const auto &tx) { return transaction_factory_->build(tx); })
      | boost::adaptors::filtered([&](const auto &result) {
          return result.match(
              [](const iroha::expected::Value<
                  std::unique_ptr<shared_model::interface::Transaction>> &) {
                return true;
              },
              [&](const iroha::expected::Error<TransportFactoryType::Error>
                      &error) {
                async_call_->log_->info(
                    "Transaction deserialization failed: hash {}, {}",
                    error.error.hash.toString(),
                    error.error.error);
                return false;
              });
        })
      | boost::adaptors::transformed([&](auto result) {
          return std::move(
                     boost::get<iroha::expected::ValueOf<decltype(result)>>(
                         result))
              .value;
        }));
}

grpc::Status MstTransportGrpc::SendState(
    ::grpc::ServerContext *context,
    const ::iroha::network::transport::MstState *request,
    ::google::protobuf::Empty *response) {
  async_call_->log_->info("MstState Received");

  auto transactions = deserializeTransactions(request);

  auto batches = batch_parser_->parseBatches(transactions);

  MstState new_state = MstState::empty();

  for (auto &batch : batches) {
    batch_factory_->createTransactionBatch(batch).match(
        [&](iroha::expected::Value<
            std::unique_ptr<shared_model::interface::TransactionBatch>>
                &value) { new_state += std::move(value).value; },
        [&](iroha::expected::Error<std::string> &error) {
          async_call_->log_->warn("Batch deserialization failed: {}",
                                  error.error);
        });
  }

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

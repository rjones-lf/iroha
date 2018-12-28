/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ordering/impl/ordering_service_transport_grpc.hpp"

#include "backend/protobuf/proposal.hpp"
#include "backend/protobuf/transaction.hpp"
#include "interfaces/common_objects/transaction_sequence_common.hpp"
#include "network/impl/grpc_channel_builder.hpp"

using namespace iroha;
using namespace iroha::ordering;

void OrderingServiceTransportGrpc::subscribe(
    std::shared_ptr<iroha::network::OrderingServiceNotification> subscriber) {
  subscriber_ = subscriber;
}

grpc::Status OrderingServiceTransportGrpc::onBatch(
    ::grpc::ServerContext *context,
    const protocol::TxList *request,
    ::google::protobuf::Empty *response) {
  async_call_->log_->info("OrderingServiceTransportGrpc::onBatch");
  if (subscriber_.expired()) {
    async_call_->log_->error("No subscriber");
  } else {
    auto txs =
        std::vector<std::shared_ptr<shared_model::interface::Transaction>>(
            request->transactions_size());
    std::transform(
        std::begin(request->transactions()),
        std::end(request->transactions()),
        std::begin(txs),
        [](const auto &tx) {
          return std::make_shared<shared_model::proto::Transaction>(tx);
        });

    // TODO [IR-1730] Akvinikym 04.10.18: use transaction factory to stateless
    // validate transactions before wrapping them into batches
    auto batch_result = batch_factory_->createTransactionBatch(txs);
    batch_result.match(
        [this](iroha::expected::Value<std::unique_ptr<
                   shared_model::interface::TransactionBatch>> &batch) {
          subscriber_.lock()->onBatch(std::move(batch.value));
        },
        [this](const iroha::expected::Error<std::string> &error) {
          async_call_->log_->error(
              "Could not create batch from received transaction list: {}",
              error.error);
        });
  }
  return ::grpc::Status::OK;
}

void OrderingServiceTransportGrpc::publishProposal(
    std::unique_ptr<shared_model::interface::Proposal> proposal,
    const std::vector<std::string> &peers) {
  async_call_->log_->info("OrderingServiceTransportGrpc::publishProposal");
  std::unordered_map<
      std::string,
      std::unique_ptr<proto::OrderingGateTransportGrpc::StubInterface>>
      peers_map;
  for (const auto &peer : peers) {
    peers_map[peer] =
        network::createClient<proto::OrderingGateTransportGrpc>(peer);
  }

  for (const auto &peer : peers_map) {
    auto proto = static_cast<shared_model::proto::Proposal *>(proposal.get());
    async_call_->log_->debug("Publishing proposal: '{}'",
                             proto->getTransport().DebugString());

    auto transport = proto->getTransport();
    async_call_->Call([&](auto context, auto cq) {
      return peer.second->AsynconProposal(context, transport, cq);
    });
  }
}

OrderingServiceTransportGrpc::OrderingServiceTransportGrpc(
    std::shared_ptr<shared_model::interface::TransactionBatchFactory>
        transaction_batch_factory,
    std::shared_ptr<network::AsyncGrpcClient<google::protobuf::Empty>>
        async_call)
    : async_call_(std::move(async_call)),
      batch_factory_(std::move(transaction_batch_factory)) {}

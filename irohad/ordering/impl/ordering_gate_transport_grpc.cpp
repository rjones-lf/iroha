/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ordering_gate_transport_grpc.hpp"

#include "backend/protobuf/transaction.hpp"
#include "endpoint.pb.h"
#include "interfaces/common_objects/types.hpp"
#include "network/impl/grpc_channel_builder.hpp"

using namespace iroha;
using namespace iroha::ordering;

grpc::Status OrderingGateTransportGrpc::onProposal(
    ::grpc::ServerContext *context,
    const iroha::protocol::Proposal *request,
    ::google::protobuf::Empty *response) {
  async_call_->log_->info("receive proposal");

  auto proposal_res = factory_->createProposal(*request);
  proposal_res.match(
      [this](iroha::expected::Value<
             std::unique_ptr<shared_model::interface::Proposal>> &v) {
        async_call_->log_->info("transactions in proposal: {}",
                                v.value->transactions().size());

        if (not subscriber_.expired()) {
          subscriber_.lock()->onProposal(std::move(v.value));
        } else {
          async_call_->log_->error("(onProposal) No subscriber");
        }
      },
      [this](const iroha::expected::Error<std::string> &e) {
        async_call_->log_->error("Received invalid proposal: {}", e.error);
      });

  return grpc::Status::OK;
}

OrderingGateTransportGrpc::OrderingGateTransportGrpc(
    const std::string &server_address,
    std::shared_ptr<network::AsyncGrpcClient<google::protobuf::Empty>>
        async_call)
    : client_(network::createClient<proto::OrderingServiceTransportGrpc>(
          server_address)),
      async_call_(std::move(async_call)),
      factory_(std::make_unique<shared_model::proto::ProtoProposalFactory<
                   shared_model::validation::DefaultProposalValidator>>()) {}

void OrderingGateTransportGrpc::propagateBatch(
    std::shared_ptr<shared_model::interface::TransactionBatch> batch) {
  async_call_->log_->info("Propagate transaction batch (on transport)");

  iroha::protocol::TxList batch_transport;
  for (const auto tx : batch->transactions()) {
    new (batch_transport.add_transactions()) iroha::protocol::Transaction(
        std::static_pointer_cast<shared_model::proto::Transaction>(tx)
            ->getTransport());
  }
  async_call_->Call([&](auto context, auto cq) {
    return client_->AsynconBatch(context, batch_transport, cq);
  });
}

void OrderingGateTransportGrpc::subscribe(
    std::shared_ptr<iroha::network::OrderingGateNotification> subscriber) {
  async_call_->log_->info("Subscribe");
  subscriber_ = subscriber;
}

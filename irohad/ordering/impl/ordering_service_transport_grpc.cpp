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
#include "interfaces/common_objects/transaction_sequence_common.hpp"
#include "interfaces/iroha_internal/transaction_batch_factory.hpp"
#include "network/impl/grpc_channel_builder.hpp"
#include "validators/default_validator.hpp"

using namespace iroha::ordering;

void OrderingServiceTransportGrpc::subscribe(
    std::shared_ptr<iroha::network::OrderingServiceNotification> subscriber) {
  subscriber_ = subscriber;
}

grpc::Status OrderingServiceTransportGrpc::onTransaction(
    ::grpc::ServerContext *context,
    const iroha::protocol::Transaction *request,
    ::google::protobuf::Empty *response) {
  async_call_->log_->info("OrderingServiceTransportGrpc::onTransaction");
  if (subscriber_.expired()) {
    async_call_->log_->error("No subscriber");
  } else {
    auto batch_result = shared_model::interface::TransactionBatchFactory::
        createTransactionBatch<
            shared_model::validation::DefaultSignedTransactionValidator>(
            std::make_shared<shared_model::proto::Transaction>(
                iroha::protocol::Transaction(*request)));
    batch_result.match(
        [this](iroha::expected::Value<shared_model::interface::TransactionBatch>
                   &batch) {
          subscriber_.lock()->onBatch(std::move(batch.value));
        },
        [this](const iroha::expected::Error<std::string> &error) {
          async_call_->log_->error(
              "Could not create batch from received single transaction: {}",
              error.error);
        });
  }

  return ::grpc::Status::OK;
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

    auto batch_result = shared_model::interface::TransactionBatchFactory::
        createTransactionBatch(
            txs,
            shared_model::validation::DefaultSignedTransactionsValidator());
    batch_result.match(
        [this](iroha::expected::Value<shared_model::interface::TransactionBatch>
                   &batch) {
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
    std::shared_ptr<network::AsyncGrpcClient<google::protobuf::Empty>>
        async_call)
    : async_call_(std::move(async_call)) {}

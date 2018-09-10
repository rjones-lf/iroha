/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ordering/impl/on_demand_os_server_grpc.hpp"

#include "backend/protobuf/proposal.hpp"

using namespace iroha::ordering::transport;

OnDemandOsServerGrpc::OnDemandOsServerGrpc(
    std::shared_ptr<OdOsNotification> ordering_service)
    : ordering_service_(ordering_service) {}

grpc::Status OnDemandOsServerGrpc::SendTransactions(
    ::grpc::ServerContext *context,
    const proto::TransactionsRequest *request,
    ::google::protobuf::Empty *response) {
  Round round{request->round().block_round(), request->round().reject_round()};
  OdOsNotification::CollectionType transactions;
  for (const auto &transaction : request->transactions()) {
    transactions.push_back(std::make_unique<shared_model::proto::Transaction>(
        iroha::protocol::Transaction(transaction)));
  }
  ordering_service_->onTransactions(round, std::move(transactions));
  return ::grpc::Status::OK;
}

grpc::Status OnDemandOsServerGrpc::RequestProposal(
    ::grpc::ServerContext *context,
    const proto::ProposalRequest *request,
    proto::ProposalResponse *response) {
  ordering_service_->onRequestProposal(
      {request->round().block_round(), request->round().reject_round()})
      | [&](auto &&proposal) {
          *response->mutable_proposal() = std::move(
              static_cast<shared_model::proto::Proposal *>(proposal.get())
                  ->getTransport());
        };
  return ::grpc::Status::OK;
}

/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <thread>

#include <grpc++/grpc++.h>

#include "common/byteutils.hpp"
#include "network/impl/grpc_channel_builder.hpp"
#include "torii/command_client.hpp"
#include "transaction.pb.h"

namespace torii {

  using iroha::protocol::ToriiResponse;
  using iroha::protocol::Transaction;

  CommandSyncClient::CommandSyncClient(const std::string &ip,
                                       size_t port,
                                       logger::Logger log)
      : ip_(ip),
        port_(port),
        stub_(iroha::network::createClient<iroha::protocol::CommandService_v1>(
            ip + ":" + std::to_string(port))),
        log_(std::move(log)) {}

  CommandSyncClient::CommandSyncClient(const CommandSyncClient &rhs)
      : CommandSyncClient(rhs.ip_, rhs.port_, rhs.log_) {}

  CommandSyncClient &CommandSyncClient::operator=(CommandSyncClient rhs) {
    swap(*this, rhs);
    return *this;
  }

  CommandSyncClient::CommandSyncClient(CommandSyncClient &&rhs) noexcept {
    swap(*this, rhs);
  }

  CommandSyncClient &CommandSyncClient::operator=(
      CommandSyncClient &&rhs) noexcept {
    swap(*this, rhs);
    return *this;
  }

  grpc::Status CommandSyncClient::Torii(const Transaction &tx) const {
    google::protobuf::Empty a;
    grpc::ClientContext context;
    return stub_->Torii(&context, tx, &a);
  }

  grpc::Status CommandSyncClient::ListTorii(
      const iroha::protocol::TxList &tx_list) const {
    google::protobuf::Empty a;
    grpc::ClientContext context;
    return stub_->ListTorii(&context, tx_list, &a);
  }

  grpc::Status CommandSyncClient::Status(
      const iroha::protocol::TxStatusRequest &request,
      iroha::protocol::ToriiResponse &response) const {
    grpc::ClientContext context;
    return stub_->Status(&context, request, &response);
  }

  void CommandSyncClient::StatusStream(
      const iroha::protocol::TxStatusRequest &tx,
      std::vector<iroha::protocol::ToriiResponse> &response) const {
    grpc::ClientContext context;
    ToriiResponse resp;
    std::unique_ptr<grpc::ClientReader<ToriiResponse> > reader(
        stub_->StatusStream(&context, tx));
    while (reader->Read(&resp)) {
      log_->debug("received new status: {}, hash {}",
                  resp.tx_status(),
                  iroha::bytestringToHexstring(resp.tx_hash()));
      response.push_back(resp);
    }
    reader->Finish();
  }

  void CommandSyncClient::swap(CommandSyncClient &lhs, CommandSyncClient &rhs) {
    using std::swap;
    swap(lhs.ip_, rhs.ip_);
    swap(lhs.port_, rhs.port_);
    swap(lhs.stub_, rhs.stub_);
    swap(lhs.log_, rhs.log_);
  }

}  // namespace torii

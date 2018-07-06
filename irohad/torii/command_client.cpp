/*
Copyright 2017 Soramitsu Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
    http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include <thread>

#include <grpc++/grpc++.h>

#include "block.pb.h"
#include "network/impl/grpc_channel_builder.hpp"
#include "common/byteutils.hpp"
#include "torii/command_client.hpp"

namespace torii {

  using iroha::protocol::ToriiResponse;
  using iroha::protocol::Transaction;

  CommandSyncClient::CommandSyncClient(const std::string &ip, size_t port)
      : ip_(ip),
        port_(port),
        stub_(iroha::network::createClient<iroha::protocol::CommandService>(
            ip + ":" + std::to_string(port))),
        log_(logger::log("CommandSyncClient")) {}

  CommandSyncClient::CommandSyncClient(const CommandSyncClient &rhs)
      : CommandSyncClient(rhs.ip_, rhs.port_) {}

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
  }

}  // namespace torii

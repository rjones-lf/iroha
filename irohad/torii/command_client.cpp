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

#include "command_client.hpp"
#include <block.pb.h>
#include <grpc++/grpc++.h>

/*
 * This client is used by peer service sending tx to change state peers.
 */
namespace api {

  using iroha::protocol::Transaction;
  using iroha::protocol::ToriiResponse;

  ToriiResponse sendTransaction(const Transaction& tx,
                                const std::string& targetPeerIp) {
    CommandClient client(targetPeerIp, 50051);  // TODO: Get port from config
    return client.Torii(tx);
  }

  CommandClient::CommandClient(const std::string& ip, int port) {
    // TODO(motxx): call validation of ip format and port.
    auto channel = grpc::CreateChannel(ip + ":" + std::to_string(port),
                                       grpc::InsecureChannelCredentials());
    stub_ = iroha::protocol::CommandService::NewStub(channel);
  }

  ToriiResponse CommandClient::Torii(const Transaction& tx) {
    ToriiResponse response;
    auto status = stub_->Torii(&context_, tx, &response);

    if (status.ok()) {
      return response;
    } else {
      response.Clear();
      response.set_code(iroha::protocol::FAIL);
      response.set_message("connection failed. cannot send transaction.");
      return response;
    }
  }

}  // namespace api

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

#include <endpoint.grpc.pb.h>
#include <grpc++/grpc++.h>

#include "client.hpp"

namespace connection {
  namespace ordering {

    using iroha::protocol::QueueTransactionResponse;
    using iroha::protocol::Transaction;

    bool send(std::string ip, const Transaction& tx) {
      // TODO
    }

    OrderingClient::OrderingClient(const std::string& ip, int port) {
      auto channel = grpc::CreateChannel(ip + ":" + std::to_string(port),
                                         grpc::InsecureChannelCredentials());
      stub_ = iroha::protocol::OrderingService::NewStub(channel);
    }

    QueueTransactionResponse OrderingClient::QueueTransaction(
        const iroha::protocol::Transaction& tx) {
      QueueTransactionResponse response;
      stub_->QueueTransaction(&context_, tx, &response);
      return response;
    }

  }  // namespace ordering
}  // namespace connection

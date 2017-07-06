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

#ifndef API_COMMAND_CLIENT_HPP
#define API_COMMAND_CLIENT_HPP

#include <endpoint.grpc.pb.h>
#include <endpoint.pb.h>

namespace api {

  iroha::protocol::ToriiResponse sendTransaction(
      const iroha::protocol::Transaction& block,
      const std::string& targetPeerIp);

  class CommandClient {
   public:
    CommandClient(const std::string& ip, int port);

    iroha::protocol::ToriiResponse Torii(
        const iroha::protocol::Transaction& request);

   private:
    grpc::ClientContext context_;
    std::unique_ptr<iroha::protocol::CommandService::Stub> stub_;
  };

}  // namespace api

#endif  // API_COMMAND_CLIENT_HPP
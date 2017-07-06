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

#ifndef CONSENSUS_CONNECTION_CLIENT_HPP
#define CONSENSUS_CONNECTION_CLIENT_HPP

#include <grpc++/grpc++.h>
#include <memory>
#include <block.pb.h>
#include <endpoint.pb.h>
#include <endpoint.grpc.pb.h>

namespace consensus {
  namespace connection {

    iroha::protocol::VerifyResponse sendBlock(
      const iroha::protocol::Block& block, const std::string& targetPeerIp);

    class SumeragiClient {
    public:
      SumeragiClient(const std::string& ip, int port);
      iroha::protocol::VerifyResponse Verify(const iroha::protocol::Block&);
    private:
      grpc::ClientContext context_;
      std::unique_ptr<iroha::protocol::SumeragiService::Stub> stub_;
    };

  }  // namespace connection
}  // namespace consensus

#endif // CONSENSUS_CONNECTION_CLIENT_HPP

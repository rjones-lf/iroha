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

#include <grpc++/grpc++.h>
#include <block.pb.h>
#include "client.hpp"

namespace consensus {
  namespace connection {

    using iroha::protocol::Block;
    using iroha::protocol::VerifyResponse;

    VerifyResponse sendBlock(const Block& block, const std::string& targetPeerIp) {
      SumeragiClient client(targetPeerIp, 50051); // TODO: Get port from config
      return client.Verify(block);
    }

    SumeragiClient::SumeragiClient(const std::string& ip, int port) {
      // TODO(motxx): call validation of ip format and port.
      auto channel = grpc::CreateChannel(ip + ":" + std::to_string(port), grpc::InsecureChannelCredentials());
      stub_ = iroha::protocol::SumeragiService::NewStub(channel);
    }

    VerifyResponse SumeragiClient::Verify(const Block& block) {
      VerifyResponse response;
      auto status = stub_->Verify(&context_, block, &response);

      if (status.ok()) {
        return response;
      } else {
        response.Clear();
        response.set_code(iroha::protocol::FAIL);
        return response;
      }
    }

  }  // namespace connection
}  // namespace consensus

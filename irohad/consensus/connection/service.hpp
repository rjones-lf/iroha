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

#ifndef CONSENSUS_CONNECTION_SERVICE_HPP
#define CONSENSUS_CONNECTION_SERVICE_HPP

#include <endpoint.grpc.pb.h>
#include <endpoint.pb.h>

namespace consensus {
  namespace connection {

    void receive(const std::function<void(const iroha::protocol::Block&)>&);

    class SumeragiService final
      : public iroha::protocol::SumeragiService::Service {
    public:
      grpc::Status Verify(
        grpc::ServerContext* context, const ::iroha::protocol::Block* request,
        iroha::protocol::VerifyResponse* response);
    };

  }  // namespace connection
}  // namespace consensus

#endif // CONSENSUS_CONNECTION_SERVICE_HPP
/**
 * Copyright Soramitsu Co., Ltd. 2017 All Rights Reserved.
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

// In iroha-cli only, " is used.
#include "client.hpp"

#include <grpc/grpc.h>
#include <grpc++/channel.h>
#include <grpc++/client_context.h>
#include <grpc++/create_channel.h>
#include <grpc++/security/credentials.h>

#include <endpoint.grpc.pb.h>
#include <endpoint.pb.h>

namespace iroha_cli {

  CliClient::CliClient(std::string targetIp, int port):
      targetIp(targetIp),
      port(port)
  {}

  std::string CliClient::sendTx(const Transaction &tx) {
    auto channel = grpc::CreateChannel(
      targetIp + ":" + std::to_string(port),
      grpc::InsecureChannelCredentials()
    );

    auto stub = iroha::protocol::CommandService::NewStub(channel);

    iroha::protocol::ToriiResponse response;
    grpc::ClientContext context;

    // ToDo model::transaction -> protocol::transaction
    iroha::protocol::Transaction transaction;
    grpc::Status status = stub->Torii(&context, transaction, &response);
    
    if (status.ok()) {
      return response.message();
    } else {
      std::cout <<
        status.error_code() << ": " <<
        status.error_message()
        << std::endl;
      return response.message();
    }
  }
};

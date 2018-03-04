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

#include "main/server_runner.hpp"

#include <boost/format.hpp>

const auto kPortBindError = "Cannot bind server to address %s";

ServerRunner::ServerRunner(const std::string &address, bool reuse)
    : serverAddress_(address), reuse_(reuse) {}

ServerRunner &ServerRunner::append(std::shared_ptr<grpc::Service> service) {
  services_.push_back(service);
  return *this;
}

iroha::expected::Result<int, std::string> ServerRunner::run() {
  grpc::ServerBuilder builder;
  int selected_port = 0;

  if (not reuse_) {
    builder.AddChannelArgument(GRPC_ARG_ALLOW_REUSEPORT, 0);
  }

  builder.AddListeningPort(
      serverAddress_, grpc::InsecureServerCredentials(), &selected_port);

  for (auto &service : services_) {
    builder.RegisterService(service.get());
  }

  serverInstance_ = builder.BuildAndStart();
  serverInstanceCV_.notify_one();

  if (selected_port == 0) {
    return iroha::expected::makeError(
        (boost::format(kPortBindError) % serverAddress_).str());
  }

  return iroha::expected::makeValue(selected_port);
}

void ServerRunner::waitForServersReady() {
  std::unique_lock<std::mutex> lock(waitForServer_);
  while (not serverInstance_) {
    serverInstanceCV_.wait(lock);
  }
}

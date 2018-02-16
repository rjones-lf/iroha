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
#include <grpc++/server.h>
#include <grpc++/server_builder.h>
#include <grpc++/server_context.h>
#include "logger/logger.hpp"

ServerRunner::ServerRunner(const std::string &address)
    : serverAddress_(address) {}

ServerRunner &ServerRunner::append(std::unique_ptr<grpc::Service> service) {
  services_.emplace_back(std::move(service));
  return *this;
}

void ServerRunner::run() {
  grpc::ServerBuilder builder;
  builder.AddListeningPort(serverAddress_, grpc::InsecureServerCredentials());

  for (auto &service : services_) {
    builder.RegisterService(service.get());
  }

  serverInstance_ = builder.BuildAndStart();
  serverInstanceCV_.notify_one();
}

void ServerRunner::shutdown() {
  serverInstance_->Shutdown();
}

void ServerRunner::waitForServersReady() {
  std::unique_lock<std::mutex> lock(waitForServer_);
  while (not serverInstance_) {
    serverInstanceCV_.wait(lock);
  }
}

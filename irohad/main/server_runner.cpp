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

#include <grpc++/server.h>
#include <grpc++/server_builder.h>
#include <grpc++/server_context.h>
#include <logger/logger.hpp>

#include "server_runner.hpp"

logger::Logger console("ServerRunner");

ServerRunner::ServerRunner(const std::string &ip, int port,
                           const std::vector<grpc::Service *> &srvs)
    : serverAddress_(ip + ":" + std::to_string(port)), services_(srvs) {}

void ServerRunner::run() {
  grpc::ServerBuilder builder;

  // TODO(motxx): Is it ok to open same port for all services?
  builder.AddListeningPort(serverAddress_, grpc::InsecureServerCredentials());
  for (auto srv : services_) {
    builder.RegisterService(srv);
  }

  waitForServer_.lock();
  serverInstance_ = builder.BuildAndStart();
  waitForServer_.unlock();
  serverInstanceCV_.notify_one();

  console.info("Server listening on {}", serverAddress_);

  serverInstance_->Wait();
}

void ServerRunner::shutdown() { serverInstance_->Shutdown(); }

bool ServerRunner::waitForServersReady() {
  std::unique_lock<std::mutex> lock(waitForServer_);
  while (!serverInstance_) serverInstanceCV_.wait(lock);
}

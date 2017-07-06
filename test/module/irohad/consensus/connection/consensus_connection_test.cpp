/*
Copyright Soramitsu Co., Ltd. 2016 All Rights Reserved.

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

#include <gtest/gtest.h>
#include <consensus/connection/service.hpp>
#include <consensus/connection/client.hpp>
#include <main/server_runner.hpp>
#include <thread>
#include <endpoint.grpc.pb.h>

namespace conn = consensus::connection;
using iroha::protocol::Block;

class ConsensusConnectionTest : public ::testing::Test {
 protected:
  virtual void SetUp() {
    serverRunner_.reset(new ServerRunner("0.0.0.0", 50051, {
      &service_
    }));
    running_ = false;
  }

  virtual void TearDown() {
    if (running_) {
      serverRunner_->shutdown();
      serverThread_.join();
    }
  }

  void RunServer() {
    serverThread_ = std::thread(&IServerRunner::run, std::ref(*serverRunner_));
    serverRunner_->waitForServersReady();
    running_ = true;
  }

 private:
  bool running_;
  conn::SumeragiService service_;
  std::unique_ptr<IServerRunner> serverRunner_;
  std::thread serverThread_;
};

/**
 * Note: Async connection is WIP.
 *       Temporarily, we tests sync connection.
 */
TEST_F(ConsensusConnectionTest, FailConnectionWhenNotStandingServer) {
  Block block;
  auto response = conn::sendBlock(block, "0.0.0.0");
  ASSERT_EQ(response.code(), iroha::protocol::ResponseCode::FAIL);
}

TEST_F(ConsensusConnectionTest, SuccessConnectionWhenStandingServer) {
  RunServer();
  Block block;
  auto response = conn::sendBlock(block, "0.0.0.0");
  ASSERT_EQ(response.code(), iroha::protocol::ResponseCode::OK);
}

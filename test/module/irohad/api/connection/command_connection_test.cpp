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

#include <endpoint.grpc.pb.h>
#include <gtest/gtest.h>
#include <api/command_client.hpp>
#include <api/command_service.hpp>
#include <main/server_runner.hpp>
#include <thread>

using iroha::protocol::Transaction;

// TODO: allow dynamic port binding in ServerRunner IR-741
class CommandConnectionTest : public ::testing::Test {
 protected:
  virtual void SetUp() {
    serverRunner_.reset(new ServerRunner("0.0.0.0", 50051, {&service_}));
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
  api::CommandService service_;
  std::unique_ptr<IServerRunner> serverRunner_;
  std::thread serverThread_;
};

/**
 * Note: Async connection is WIP.
 *       Temporarily, we tests sync connection.
 */
TEST_F(CommandConnectionTest, FailConnectionWhenNotStandingServer) {
  Transaction tx;
  auto response = api::sendTransaction(tx, "0.0.0.0");
  ASSERT_EQ(response.code(), iroha::protocol::ResponseCode::FAIL);
  ASSERT_STREQ(response.message().c_str(),
               "connection failed. cannot send transaction.");
}

TEST_F(CommandConnectionTest, SuccessConnectionWhenStandingServer) {
  RunServer();

  Transaction tx;
  auto response = api::sendTransaction(tx, "0.0.0.0");
  ASSERT_EQ(response.code(), iroha::protocol::ResponseCode::FAIL);
  ASSERT_STREQ(response.message().c_str(), "failed stateless validation.");
}

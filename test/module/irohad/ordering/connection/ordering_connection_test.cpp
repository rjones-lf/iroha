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
#include <ordering/connection/client.hpp>
#include <ordering/connection/service.hpp>
#include <ordering/observer.hpp>
#include <main/server_runner.hpp>
#include <thread>
#include <endpoint.grpc.pb.h>
#include <peer_service/self_state.hpp>

using iroha::protocol::Transaction;

class OrderingConnectionTest : public ::testing::Test {
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
  ordering::connection::OrderingService service_;
  std::unique_ptr<IServerRunner> serverRunner_;
  std::thread serverThread_;
};

/**
 * fails connection because server isn't running.
 */
TEST_F(OrderingConnectionTest, FailConnectionWhenNotRunningServer) {
  Transaction tx;
  auto response = ordering::connection::sendTransaction(tx, peer_service::self_state::getIp());
  ASSERT_EQ(response.code(), iroha::protocol::ResponseCode::FAIL);
}

/**
 * success connection, but fails stateful validation.
 */
TEST_F(OrderingConnectionTest, SuccessConnectionWhenRunningServer) {
  RunServer();

  ordering::observer::initialize();
  Transaction tx;
  auto response = ordering::connection::sendTransaction(tx, peer_service::self_state::getIp());
  ASSERT_EQ(response.code(), iroha::protocol::ResponseCode::OK);
}

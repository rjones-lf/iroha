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

#include "module/irohad/ordering/ordering_mocks.hpp"

#include <grpc++/grpc++.h>
#include <logger/logger.hpp>
#include "ordering/impl/ordering_service_impl.hpp"
#include "module/irohad/ametsuchi/ametsuchi_mocks.hpp"

using namespace iroha;
using namespace iroha::ordering;
using namespace iroha::model;
using namespace iroha::network;
using namespace iroha::ametsuchi;
using namespace std::chrono_literals;

using ::testing::_;
using ::testing::Invoke;
using ::testing::AtLeast;
using ::testing::Return;

static logger::Logger log_ = logger::testLog("OrderingService");

class OrderingServiceTest : public OrderingTest {
 public:
  OrderingServiceTest() {
    fake_gate = static_cast<ordering::MockOrderingGate *>(gate.get());
  }

  void SetUp() override { loop = uvw::Loop::create(); }

  void start() override {
    OrderingTest::start();
    loop_thread = std::thread([this] { loop->run(); });
    client = proto::OrderingService::NewStub(
        grpc::CreateChannel(address, grpc::InsecureChannelCredentials()));
  }

  void set_watchdog(std::chrono::milliseconds d) {
    watchdog_timeout_ = true;
    t_ = std::make_shared<std::thread>([this, d]() {
      std::mutex _m;
      std::unique_lock<std::mutex> _lk(_m);
      cv_.wait_for(_lk, d);
      log_->info("Timeout reached");
      ASSERT_FALSE(watchdog_timeout_);
    });
  }

  void end_watchdog() {
    watchdog_timeout_ = false;
    cv_.notify_one();
    t_->join();
  }

  void send() {
    grpc::ClientContext context;
    google::protobuf::Empty reply;
    client->SendTransaction(&context, iroha::protocol::Transaction(), &reply);
  }

  void shutdown() override {
    loop->stop();
    if (loop_thread.joinable()) {
      loop_thread.join();
    }
    OrderingTest::shutdown();
  }

  std::shared_ptr<uvw::Loop> loop;
  std::thread loop_thread;
  ordering::MockOrderingGate *fake_gate;
  std::unique_ptr<iroha::ordering::proto::OrderingService::Stub> client;
  std::condition_variable cv;
  std::mutex m;

 private:
  std::atomic_bool watchdog_timeout_;
  std::condition_variable cv_;
  std::shared_ptr<std::thread> t_;
};

TEST_F(OrderingServiceTest, ValidWhenProposalSizeStrategy) {
  // Init => proposal size 5 => 2 proposals after 10 transactions

  std::shared_ptr<MockPeerQuery> wsv = std::make_shared<MockPeerQuery>();
  EXPECT_CALL(*wsv, getLedgerPeers()).WillRepeatedly(Return(std::vector<Peer>{
      peer}));
  
  const size_t max_proposal = 5;
  const size_t commit_delay = 1000;
  service = std::make_shared<OrderingServiceImpl>(wsv, max_proposal, commit_delay, loop);

  EXPECT_CALL(*fake_gate, SendProposal(_, _, _)).Times(2);
  ON_CALL(*fake_gate, SendProposal(_, _, _))
      .WillByDefault(Invoke([this](auto, auto, auto) {
        cv.std::condition_variable::notify_one();
        log_->info("Proposal send to grpc");
        return grpc::Status::OK;
      }));

  start();
  set_watchdog(8s);

  std::unique_lock<std::mutex> lk(m);
  for (size_t i = 0; i < 10; ++i) {
    send();
    if (i != 0 && i % max_proposal == 0) {
      cv.wait_for(lk, 10s);
    }
  }

  end_watchdog();
}

TEST_F(OrderingServiceTest, ValidWhenTimerStrategy) {
  // Init => proposal timer 400 ms => 10 tx by 50 ms => 2 proposals in 1 second

  std::shared_ptr<MockPeerQuery> wsv = std::make_shared<MockPeerQuery>();
  EXPECT_CALL(*wsv, getLedgerPeers()).WillRepeatedly(Return(std::vector<Peer>{
      peer}));
  
  const size_t max_proposal = 100;
  const size_t commit_delay = 400;
  service = std::make_shared<OrderingServiceImpl>(wsv, max_proposal, commit_delay, loop);

  EXPECT_CALL(*fake_gate, SendProposal(_, _, _)).Times(2);
  ON_CALL(*fake_gate, SendProposal(_, _, _))
      .WillByDefault(Invoke([this](auto, auto, auto) {
        cv.std::condition_variable::notify_one();
        return grpc::Status::OK;
      }));

  start();
  set_watchdog(8s);

  for (size_t i = 0; i < 8; ++i) {
    send();
  }

  std::unique_lock<std::mutex> lk(m);
  cv.wait_for(lk, 10s);

  send(); send();
  cv.wait_for(lk, 10s);
  end_watchdog();
}

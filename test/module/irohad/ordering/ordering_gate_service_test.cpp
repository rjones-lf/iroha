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

#include "framework/test_subscriber.hpp"
#include "ordering/impl/ordering_gate_impl.hpp"
#include "ordering/impl/ordering_service_impl.hpp"
#include "ordering_mocks.hpp"
#include "module/irohad/ametsuchi/ametsuchi_mocks.hpp"

using namespace iroha::ordering;
using namespace iroha::model;
using namespace iroha::network;
using namespace framework::test_subscriber;
using namespace iroha::ametsuchi;
using namespace std::chrono_literals;
using ::testing::Return;

class OrderingGateServiceTest : public OrderingTest {
 public:
  OrderingGateServiceTest() {
    gate_impl = std::make_shared<OrderingGateImpl>(address);
    gate = gate_impl;
    counter = 2;
  }

  void SetUp() override { loop = uvw::Loop::create(); }

  void start() override {
    OrderingTest::start();
    loop_thread = std::thread([this] { loop->run(); });
  }

  void shutdown() override {
    proposals.clear();
    loop->stop();
    if (loop_thread.joinable()) {
      loop_thread.join();
    }
    OrderingTest::shutdown();
  }

  TestSubscriber<iroha::model::Proposal> init(size_t times) {
    auto wrapper = make_test_subscriber<CallExact>(gate_impl->on_proposal(), times);
    wrapper.subscribe([this](auto proposal) { proposals.push_back(proposal); });
    gate_impl->on_proposal().subscribe([this](auto) {
      counter--;
      cv.notify_one();
    });
    return wrapper;
  }

  void send_transaction(size_t i) {
    auto tx = std::make_shared<Transaction>();
    tx->tx_counter = i;
    gate_impl->propagate_transaction(tx);
    // otherwise tx may come unordered
    std::this_thread::sleep_for(1ms);
  }

  std::shared_ptr<uvw::Loop> loop;
  std::thread loop_thread;
  std::shared_ptr<OrderingGateImpl> gate_impl;
  std::vector<Proposal> proposals;
  std::atomic<size_t> counter;
  std::condition_variable cv;
  std::mutex m;
};

TEST_F(OrderingGateServiceTest, SplittingBunchTransactions) {
  // 8 transaction -> proposal -> 2 transaction -> proposal

  std::shared_ptr<MockPeerQuery> wsv = std::make_shared<MockPeerQuery>();
  EXPECT_CALL(*wsv, getLedgerPeers()).WillRepeatedly(Return(std::vector<Peer>{
      peer}));
  const size_t max_proposal = 100;
  const size_t commit_delay = 400;

  service = std::make_shared<OrderingServiceImpl>(wsv,
                                                  max_proposal,
                                                  commit_delay,
                                                  loop);

  start();
  std::unique_lock<std::mutex> lk(m);
  auto wrapper = init(2);

  for (size_t i = 0; i < 8; ++i) {
    send_transaction(i);
  }

  cv.wait_for(lk, 10s);
  send_transaction(8); send_transaction(9);
  cv.wait_for(lk, 10s);

  ASSERT_TRUE(wrapper.validate());
  ASSERT_EQ(proposals.size(), 2);
  ASSERT_EQ(proposals.at(0).transactions.size(), 8);
  ASSERT_EQ(proposals.at(1).transactions.size(), 2);
  ASSERT_EQ(counter, 0);

  size_t i = 0;
  for (auto &&proposal : proposals) {
    for (auto &&tx : proposal.transactions) {
      ASSERT_EQ(tx.tx_counter, i++);
    }
  }
}

TEST_F(OrderingGateServiceTest, ProposalsReceivedWhenProposalSize) {
  // commits on the fulfilling proposal queue
  // 10 transaction -> proposal with 5 -> proposal with 5

  std::shared_ptr<MockPeerQuery> wsv = std::make_shared<MockPeerQuery>();
  EXPECT_CALL(*wsv, getLedgerPeers()).WillRepeatedly(Return(std::vector<Peer>{
      peer}));
  const size_t max_proposal = 5;
  const size_t commit_delay = 1000;

  service = std::make_shared<OrderingServiceImpl>(wsv, max_proposal,
                                                  commit_delay, loop);

  start();
  std::unique_lock<std::mutex> lk(m);
  auto wrapper = init(2);

  for (size_t i = 0; i < 10; ++i) {
    send_transaction(i);
  }

  // long == something wrong
  cv.wait_for(lk, 10s, [this]() { return counter == 0; });

  ASSERT_TRUE(wrapper.validate());
  ASSERT_EQ(proposals.size(), 2);
  ASSERT_EQ(counter, 0);

  size_t i = 0;
  for (auto &&proposal : proposals) {
    ASSERT_EQ(proposal.transactions.size(), 5);
    for (auto &&tx : proposal.transactions) {
      ASSERT_EQ(tx.tx_counter, i++);
    }
  }
}

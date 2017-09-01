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
#include "ordering/impl/ordering_gate_transport_grpc.hpp"
#include "ordering/impl/ordering_service_transport_grpc.hpp"

using namespace iroha::ordering;
using namespace iroha::model;
using namespace iroha::network;
using namespace framework::test_subscriber;
using namespace iroha::ametsuchi;
using ::testing::Return;

class OrderingGateServiceTest : public OrderingTest {
 public:
  OrderingGateServiceTest() {
    auto transport = std::make_shared<OrderingGateTransportgRPC>(address);
    gate_impl = std::make_shared<OrderingGateImpl>(transport);
    gate_impl->transport_->subscribe(gate_impl);
    gate = gate_impl;
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

  std::shared_ptr<uvw::Loop> loop;
  std::thread loop_thread;
  std::shared_ptr<OrderingGateImpl> gate_impl;
  std::vector<Proposal> proposals;
};

TEST_F(OrderingGateServiceTest, ProposalsReceivedWhenTimer) {
  // todo write use case

  std::shared_ptr<MockPeerQuery> wsv = std::make_shared<MockPeerQuery>();
  EXPECT_CALL(*wsv, getLedgerPeers()).WillRepeatedly(Return(std::vector<Peer>{
      peer}));

  auto transport = std::make_shared<OrderingServiceTransportGrpc>();
  service = std::make_shared<OrderingServiceImpl>(wsv,
                                                  100,
                                                  400,
                                                  transport,
                                                  loop);

  start();

  auto wrapper = make_test_subscriber<CallExact>(gate_impl->on_proposal(), 2);
  wrapper.subscribe([this](auto proposal) { proposals.push_back(proposal); });

  for (size_t i = 0; i < 10; ++i) {
    auto tx = std::make_shared<Transaction>();
    tx->tx_counter = i;
    gate_impl->propagate_transaction(tx);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }

  std::this_thread::sleep_for(std::chrono::seconds(1));

  ASSERT_TRUE(wrapper.validate());
  ASSERT_EQ(proposals.size(), 2);
  ASSERT_EQ(proposals.at(0).transactions.size(), 8);
  ASSERT_EQ(proposals.at(1).transactions.size(), 2);

  size_t i = 0;
  for (auto &&proposal : proposals) {
    for (auto &&tx : proposal.transactions) {
      ASSERT_EQ(tx.tx_counter, i++);
    }
  }
}

TEST_F(OrderingGateServiceTest, ProposalsReceivedWhenProposalSize) {
  // todo write use case

  std::shared_ptr<MockPeerQuery> wsv = std::make_shared<MockPeerQuery>();
  EXPECT_CALL(*wsv, getLedgerPeers()).WillRepeatedly(Return(std::vector<Peer>{
      peer}));

  auto transport = std::make_shared<OrderingServiceTransportGrpc>();
  service = std::make_shared<OrderingServiceImpl>(wsv, 5, 1000,
                                                  transport, loop);

  start();

  auto wrapper = make_test_subscriber<CallExact>(gate_impl->on_proposal(), 2);
  wrapper.subscribe([this](auto proposal) { proposals.push_back(proposal); });

  for (size_t i = 0; i < 10; ++i) {
    auto tx = std::make_shared<Transaction>();
    tx->tx_counter = i;
    gate_impl->propagate_transaction(tx);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }

  std::this_thread::sleep_for(std::chrono::milliseconds(500));

  ASSERT_TRUE(wrapper.validate());
  ASSERT_EQ(proposals.size(), 2);

  size_t i = 0;
  for (auto &&proposal : proposals) {
    ASSERT_EQ(proposal.transactions.size(), 5);
    for (auto &&tx : proposal.transactions) {
      ASSERT_EQ(tx.tx_counter, i++);
    }
  }
}

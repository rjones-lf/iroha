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
#include "ordering/impl/ordering_service_impl.hpp"
#include "module/irohad/ametsuchi/ametsuchi_mocks.hpp"
#include "ordering/impl/ordering_service_transport_grpc.hpp"

using namespace iroha;
using namespace iroha::ordering;
using namespace iroha::model;
using namespace iroha::network;
using namespace iroha::ametsuchi;

using ::testing::_;
using ::testing::AtLeast;
using ::testing::Return;

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
};

TEST_F(OrderingServiceTest, ValidWhenProposalSizeStrategy) {
  // Init => proposal size 5 => 2 proposals after 10 transactions

  std::shared_ptr<MockPeerQuery> wsv = std::make_shared<MockPeerQuery>();
  EXPECT_CALL(*wsv, getLedgerPeers()).WillRepeatedly(Return(std::vector<Peer>{
      peer}));

  auto transport = std::make_shared<OrderingServiceTransportGrpc>();
  service = std::make_shared<OrderingServiceImpl>(wsv, 5, 1000, transport, loop);

  EXPECT_CALL(*fake_gate, SendProposal(_, _, _)).Times(2);
  start();

  for (size_t i = 0; i < 10; ++i) {
    grpc::ClientContext context;

    google::protobuf::Empty reply;

    client->SendTransaction(&context, iroha::protocol::Transaction(), &reply);

  }

  std::this_thread::sleep_for(std::chrono::seconds(1));
}

TEST_F(OrderingServiceTest, ValidWhenTimerStrategy) {
  // Init => proposal timer 400 ms => 10 tx by 50 ms => 2 proposals in 1 second

  std::shared_ptr<MockPeerQuery> wsv = std::make_shared<MockPeerQuery>();
  EXPECT_CALL(*wsv, getLedgerPeers()).WillRepeatedly(Return(std::vector<Peer>{
      peer}));

  auto transport = std::make_shared<OrderingServiceTransportGrpc>();
  service = std::make_shared<OrderingServiceImpl>(wsv, 100, 400, transport, loop);

  EXPECT_CALL(*fake_gate, SendProposal(_, _, _)).Times(2);

  start();

  for (size_t i = 0; i < 10; ++i) {
    grpc::ClientContext context;

    google::protobuf::Empty reply;

    client->SendTransaction(&context, iroha::protocol::Transaction(), &reply);

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }

  std::this_thread::sleep_for(std::chrono::seconds(1));
}

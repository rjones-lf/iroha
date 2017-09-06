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

#include "framework/test_subscriber.hpp"
#include "ordering/impl/ordering_gate_impl.hpp"
#include "ordering/impl/ordering_gate_transport_grpc.hpp"

using namespace iroha::ordering;
using namespace iroha::model;
using namespace iroha::network;
using namespace framework::test_subscriber;

using ::testing::_;

class OrderingGateTest : public OrderingTest {
 public:
  OrderingGateTest() {
    auto transport = std::make_shared<OrderingGateTransportGrpc>(address);
    gate_impl = std::make_shared<OrderingGateImpl>(transport);
    gate_impl->transport_->subscribe(gate_impl);

    gate = gate_impl;
    fake_service = static_cast<MockOrderingService*>(service.get());


  }

  std::shared_ptr<OrderingGateImpl> gate_impl;
  MockOrderingService* fake_service;
};

TEST_F(OrderingGateTest, TransactionReceivedByServerWhenSent) {
  // Init => send 5 transactions => 5 transactions are processed by server
  EXPECT_CALL(*fake_service, SendTransaction(_, _, _)).Times(5);

  for (size_t i = 0; i < 5; ++i) {
    gate_impl->propagate_transaction(std::make_shared<Transaction>());
  }

  // Ensure that server processed the transactions
  std::this_thread::sleep_for(std::chrono::seconds(1));
}

TEST_F(OrderingGateTest, ProposalReceivedByGateWhenSent) {
  auto wrapper = make_test_subscriber<CallExact>(gate_impl->on_proposal(), 1);
  wrapper.subscribe();

  grpc::ServerContext context;
  iroha::ordering::proto::Proposal proposal;

  google::protobuf::Empty response;
  gate_impl->SendProposal(&context, &proposal, &response);

  ASSERT_TRUE(wrapper.validate());
}

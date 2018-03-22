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
#include <grpc++/grpc++.h>
#include <gtest/gtest.h>

#include "framework/test_subscriber.hpp"

#include "module/irohad/network/network_mocks.hpp"
#include "network/ordering_service.hpp"

#include "ordering/impl/ordering_gate_impl.hpp"
#include "ordering/impl/ordering_gate_transport_grpc.hpp"
#include "ordering/impl/ordering_service_impl.hpp"
#include "ordering/impl/ordering_service_transport_grpc.hpp"

#include "module/shared_model/builders/protobuf/test_block_builder.hpp"

using namespace iroha;
using namespace iroha::ordering;
using namespace iroha::network;
using namespace framework::test_subscriber;
using namespace std::chrono_literals;

using ::testing::_;
using ::testing::InvokeWithoutArgs;
using ::testing::Return;

class MockOrderingGateTransportGrpcService
    : public proto::OrderingServiceTransportGrpc::Service {
 public:
  MOCK_METHOD3(onTransaction,
               ::grpc::Status(::grpc::ServerContext *,
                              const iroha::protocol::Transaction *,
                              ::google::protobuf::Empty *));
};

class MockOrderingGateTransport : public OrderingGateTransport {
  MOCK_METHOD1(subscribe, void(std::shared_ptr<OrderingGateNotification>));
  MOCK_METHOD1(propagateTransaction,
               void(std::shared_ptr<const iroha::model::Transaction>));
};

class OrderingGateTest : public ::testing::Test {
 public:
  OrderingGateTest()
      : fake_service{std::make_shared<MockOrderingGateTransportGrpcService>()} {
  }

  void SetUp() override {
    thread = std::thread([this] {
      grpc::ServerBuilder builder;
      int port = 0;
      builder.AddListeningPort(
          "0.0.0.0:0", grpc::InsecureServerCredentials(), &port);

      builder.RegisterService(fake_service.get());

      server = builder.BuildAndStart();
      auto address = "0.0.0.0:" + std::to_string(port);
      // Initialize components after port has been bind
      transport = std::make_shared<OrderingGateTransportGrpc>(address);
      gate_impl = std::make_shared<OrderingGateImpl>(transport);
      transport->subscribe(gate_impl);

      ASSERT_NE(port, 0);
      ASSERT_TRUE(server);
      cv.notify_one();
      server->Wait();
    });

    std::unique_lock<std::mutex> lock(m);
    cv.wait_for(lock, std::chrono::seconds(1));
  }

  void TearDown() override {
    server->Shutdown();
    if (thread.joinable()) {
      thread.join();
    }
  }

  std::unique_ptr<grpc::Server> server;

  std::shared_ptr<OrderingGateTransportGrpc> transport;
  std::shared_ptr<OrderingGateImpl> gate_impl;
  std::shared_ptr<MockOrderingGateTransportGrpcService> fake_service;
  std::thread thread;
  std::condition_variable cv;
  std::mutex m;
};

/**
 * @given Initialized OrderingGate
 * @when  Send 5 transactions to Ordering Gate
 * @then  Check that transactions are received
 */
TEST_F(OrderingGateTest, TransactionReceivedByServerWhenSent) {
  size_t call_count = 0;
  EXPECT_CALL(*fake_service, onTransaction(_, _, _))
      .Times(5)
      .WillRepeatedly(InvokeWithoutArgs([&] {
        ++call_count;
        cv.notify_one();
        return grpc::Status::OK;
      }));

  for (size_t i = 0; i < 5; ++i) {
    gate_impl->propagateTransaction(std::make_shared<iroha::model::Transaction>());
  }

  std::unique_lock<std::mutex> lock(m);
  cv.wait_for(lock, 10s, [&] { return call_count == 5; });
}

/**
 * @given Initialized OrderingGate
 * @when  Emulation of receiving proposal from the network
 * @then  Round starts <==> proposal is emitted to subscribers
 */
TEST_F(OrderingGateTest, ProposalReceivedByGateWhenSent) {
  auto wrapper = make_test_subscriber<CallExact>(gate_impl->on_proposal(), 1);
  wrapper.subscribe();

  grpc::ServerContext context;
  iroha::protocol::Proposal proposal;

  google::protobuf::Empty response;

  transport->onProposal(&context, &proposal, &response);

  ASSERT_TRUE(wrapper.validate());
}

/**
 * @given Initialized OrderingGate
 *        AND MockPeerCommunicationService
 * @when  Send two proposals
 *        AND one commit in node
 * @then  Check that send round appears after commit
 */
TEST(OrderingGateQueueBehaviour, SendManyProposals) {
  std::shared_ptr<OrderingGateTransport> transport =
      std::make_shared<MockOrderingGateTransport>();

  std::shared_ptr<MockPeerCommunicationService> pcs =
      std::make_shared<MockPeerCommunicationService>();
  rxcpp::subjects::subject<Commit> commit_subject;
  EXPECT_CALL(*pcs, on_commit())
      .WillOnce(Return(commit_subject.get_observable()));

  OrderingGateImpl ordering_gate(transport);
  ordering_gate.setPcs(*pcs);

  auto wrapper_before =
      make_test_subscriber<CallExact>(ordering_gate.on_proposal(), 1);
  wrapper_before.subscribe();
  auto wrapper_after =
      make_test_subscriber<CallExact>(ordering_gate.on_proposal(), 2);
  wrapper_after.subscribe();

  ordering_gate.onProposal(iroha::model::Proposal{{}});
  ordering_gate.onProposal(iroha::model::Proposal{{}});

  ASSERT_TRUE(wrapper_before.validate());

  std::shared_ptr<shared_model::interface::Block> block =
      std::make_shared<shared_model::proto::Block>(TestBlockBuilder().build());

  commit_subject.get_subscriber().on_next(rxcpp::observable<>::just(block));

  ASSERT_TRUE(wrapper_after.validate());
}

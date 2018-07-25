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

#include "builders/protobuf/proposal.hpp"
#include "builders/protobuf/transaction.hpp"
#include "module/irohad/network/network_mocks.hpp"
#include "module/shared_model/builders/protobuf/test_block_builder.hpp"
#include "module/shared_model/builders/protobuf/test_proposal_builder.hpp"
#include "module/shared_model/builders/protobuf/test_transaction_builder.hpp"
#include "ordering/impl/ordering_gate_impl.hpp"
#include "ordering/impl/ordering_gate_transport_grpc.hpp"

using namespace iroha;
using namespace iroha::ordering;
using namespace iroha::network;
using namespace framework::test_subscriber;
using namespace std::chrono_literals;

using ::testing::_;
using ::testing::InvokeWithoutArgs;
using ::testing::Return;

using shared_model::interface::types::HeightType;

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
  MOCK_METHOD1(
      propagateTransaction,
      void(std::shared_ptr<const shared_model::interface::Transaction>));
  MOCK_METHOD1(propagateBatch,
               void(const shared_model::interface::TransactionBatch &));
};

class OrderingGateTest : public ::testing::Test {
 public:
  OrderingGateTest()
      : fake_service{std::make_shared<MockOrderingGateTransportGrpcService>()} {
  }

  void SetUp() override {
    grpc::ServerBuilder builder;
    int port = 0;
    builder.AddListeningPort(
        "0.0.0.0:0", grpc::InsecureServerCredentials(), &port);

    builder.RegisterService(fake_service.get());

    server = builder.BuildAndStart();
    auto address = "0.0.0.0:" + std::to_string(port);
    // Initialize components after port has been bind
    transport = std::make_shared<OrderingGateTransportGrpc>(address);
    gate_impl = std::make_shared<OrderingGateImpl>(transport, 1, false);
    transport->subscribe(gate_impl);

    ASSERT_NE(port, 0);
    ASSERT_TRUE(server);
  }

  void TearDown() override {
    server->Shutdown();
  }

  std::unique_ptr<grpc::Server> server;

  std::shared_ptr<OrderingGateTransportGrpc> transport;
  std::shared_ptr<OrderingGateImpl> gate_impl;
  std::shared_ptr<MockOrderingGateTransportGrpcService> fake_service;
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
    auto tx = std::make_shared<shared_model::proto::Transaction>(
        TestTransactionBuilder().build());
    gate_impl->propagateTransaction(tx);
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

  auto pcs = std::make_shared<MockPeerCommunicationService>();
  rxcpp::subjects::subject<Commit> commit_subject;
  EXPECT_CALL(*pcs, on_commit())
      .WillOnce(Return(commit_subject.get_observable()));
  gate_impl->setPcs(*pcs);

  grpc::ServerContext context;
  auto tx = shared_model::proto::TransactionBuilder()
                .createdTime(iroha::time::now())
                .creatorAccountId("admin@ru")
                .addAssetQuantity("coin#coin", "1.0")
                .quorum(1)
                .build()
                .signAndAddSignature(
                    shared_model::crypto::DefaultCryptoAlgorithmType::
                        generateKeypair())
                .finish();
  std::vector<shared_model::proto::Transaction> txs = {tx, tx};
  iroha::protocol::Proposal proposal = shared_model::proto::ProposalBuilder()
                                           .height(2)
                                           .createdTime(iroha::time::now())
                                           .transactions(txs)
                                           .build()
                                           .getTransport();

  google::protobuf::Empty response;

  transport->onProposal(&context, &proposal, &response);

  ASSERT_TRUE(wrapper.validate());
}

class QueueBehaviorTest : public ::testing::Test {
 public:
  QueueBehaviorTest() : ordering_gate(transport, 1, false){};

  void SetUp() override {
    transport = std::make_shared<MockOrderingGateTransport>();
    pcs = std::make_shared<MockPeerCommunicationService>();
    EXPECT_CALL(*pcs, on_commit())
        .WillOnce(Return(commit_subject.get_observable()));

    ordering_gate.setPcs(*pcs);
    ordering_gate.on_proposal().subscribe(
        [&](auto val) { messages.push_back(val); });
  }

  std::shared_ptr<MockOrderingGateTransport> transport;
  std::shared_ptr<MockPeerCommunicationService> pcs;
  rxcpp::subjects::subject<Commit> commit_subject;
  OrderingGateImpl ordering_gate;
  std::vector<decltype(ordering_gate.on_proposal())::value_type> messages;

  void pushCommit(HeightType height) {
    commit_subject.get_subscriber().on_next(rxcpp::observable<>::just(
        std::static_pointer_cast<shared_model::interface::Block>(
            std::make_shared<shared_model::proto::Block>(
                TestBlockBuilder().height(height).build()))));
  }

  void pushProposal(HeightType height) {
    ordering_gate.onProposal(std::make_shared<shared_model::proto::Proposal>(
        TestProposalBuilder().height(height).build()));
  };
};

/**
 * @given Initialized OrderingGate
 *        AND MockPeerCommunicationService
 * @when  Send two proposals
 *        AND one commit in node
 * @then  Check that send round appears after commit
 */
TEST_F(QueueBehaviorTest, SendManyProposals) {
  auto wrapper_before =
      make_test_subscriber<CallExact>(ordering_gate.on_proposal(), 1);
  wrapper_before.subscribe();
  auto wrapper_after =
      make_test_subscriber<CallExact>(ordering_gate.on_proposal(), 2);
  wrapper_after.subscribe();

  auto tx = shared_model::proto::TransactionBuilder()
                .createdTime(iroha::time::now())
                .creatorAccountId("admin@ru")
                .addAssetQuantity("coin#coin", "1.0")
                .quorum(1)
                .build()
                .signAndAddSignature(
                    shared_model::crypto::DefaultCryptoAlgorithmType::
                        generateKeypair())
                .finish();
  std::vector<shared_model::proto::Transaction> txs = {tx, tx};
  auto proposal1 = std::make_shared<shared_model::proto::Proposal>(
      shared_model::proto::ProposalBuilder()
          .height(2)
          .createdTime(iroha::time::now())
          .transactions(txs)
          .build());
  auto proposal2 = std::make_shared<shared_model::proto::Proposal>(
      shared_model::proto::ProposalBuilder()
          .height(3)
          .createdTime(iroha::time::now())
          .transactions(txs)
          .build());

  ordering_gate.onProposal(proposal1);
  ordering_gate.onProposal(proposal2);

  ASSERT_TRUE(wrapper_before.validate());

  std::shared_ptr<shared_model::interface::Block> block =
      std::make_shared<shared_model::proto::Block>(
          TestBlockBuilder().height(2).build());

  commit_subject.get_subscriber().on_next(rxcpp::observable<>::just(block));

  ASSERT_TRUE(wrapper_after.validate());
}

/**
 * @given Initialized OrderingGate
 * AND MockPeerCommunicationService
 * @when Receive proposals in random order
 * @then on_proposal output is ordered
 */
TEST_F(QueueBehaviorTest, ReceiveUnordered) {
  // this will set unlock_next_ to false, so proposals 3 and 4 are enqueued
  pushProposal(2);

  pushProposal(4);
  pushProposal(3);

  pushCommit(2);
  pushCommit(3);

  ASSERT_EQ(3, messages.size());
  ASSERT_EQ(2, messages.at(0)->height());
  ASSERT_EQ(3, messages.at(1)->height());
  ASSERT_EQ(4, messages.at(2)->height());
}

/**
 * @given Initialized OrderingGate
 * AND MockPeerCommunicationService
 * @when Receive commits which are newer than existing proposals
 * @then on_proposal is not invoked on proposals
 * which are older than last committed block
 */
TEST_F(QueueBehaviorTest, DiscardOldProposals) {
  pushProposal(2);
  pushProposal(3);

  pushProposal(4);
  pushProposal(5);
  pushCommit(4);

  // proposals 2 and 3 must not be forwarded down the pipeline.
  EXPECT_EQ(2, messages.size());
  ASSERT_EQ(2, messages.at(0)->height());
  ASSERT_EQ(5, messages.at(1)->height());
}

/**
 * @given Initialized OrderingGate
 * AND MockPeerCommunicationService
 * @when Proposals are newer than received commits
 * @then newer proposals are kept in queue
 */
TEST_F(QueueBehaviorTest, KeepNewerProposals) {
  pushProposal(2);
  pushProposal(3);
  pushProposal(4);

  pushCommit(2);

  // proposal 3 must  be forwarded down the pipeline, 4 kept in queue.
  EXPECT_EQ(2, messages.size());
  EXPECT_EQ(2, messages.at(0)->height());
  EXPECT_EQ(3, messages.at(1)->height());

  pushCommit(3);
  // Now proposal 4 is forwarded to the pipeline
  EXPECT_EQ(3, messages.size());
  EXPECT_EQ(4, messages.at(2)->height());
}

/**
 * @given Initialized OrderingGate
 * AND MockPeerCommunicationService
 * @when commit is received before any proposals
 * @then old proposals are discarded and new is propagated
 */
TEST_F(QueueBehaviorTest, CommitBeforeProposal) {
  pushCommit(4);

  // Old proposals should be discarded
  pushProposal(2);
  pushProposal(3);
  pushProposal(4);

  EXPECT_EQ(0, messages.size());

  // should be propagated
  pushProposal(5);

  // should not be propagated
  pushProposal(6);

  EXPECT_EQ(1, messages.size());
  EXPECT_EQ(5, messages.at(0)->height());
}

/**
 * @given Initialized OrderingGate
 * AND MockPeerCommunicationService
 * @when commit is received which newer than all proposals
 * @then all proposals are discarded and none are propagated
 */
TEST_F(QueueBehaviorTest, CommitNewerThanAllProposals) {
  pushProposal(2);
  // Old proposals should be discarded
  pushProposal(3);
  pushProposal(4);

  pushCommit(4);
  EXPECT_EQ(1, messages.size());
  EXPECT_EQ(2, messages.at(0)->height());
}

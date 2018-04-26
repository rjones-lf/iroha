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

#include <gtest/gtest.h>

#include "builders/protobuf/common_objects/proto_peer_builder.hpp"
#include "builders/protobuf/transaction.hpp"
#include "framework/test_subscriber.hpp"
#include "mock_ordering_service_persistent_state.hpp"
#include "module/irohad/ametsuchi/ametsuchi_mocks.hpp"
#include "module/irohad/network/network_mocks.hpp"
#include "module/shared_model/builders/protobuf/test_block_builder.hpp"
#include "module/shared_model/builders/protobuf/test_transaction_builder.hpp"
#include "ordering/impl/ordering_gate_impl.hpp"
#include "ordering/impl/ordering_gate_transport_grpc.hpp"
#include "ordering/impl/ordering_service_impl.hpp"
#include "ordering/impl/ordering_service_transport_grpc.hpp"

using namespace iroha;
using namespace iroha::ordering;
using namespace iroha::network;
using namespace framework::test_subscriber;
using namespace iroha::ametsuchi;
using namespace std::chrono_literals;

using ::testing::Return;

using wPeer = std::shared_ptr<shared_model::interface::Peer>;

// TODO: refactor services to allow dynamic port binding IR-741
class OrderingGateServiceTest : public ::testing::Test {
 public:
  OrderingGateServiceTest() {
    peer = clone(shared_model::proto::PeerBuilder()
                     .address(address)
                     .pubkey(shared_model::interface::types::PubkeyType(
                         std::string(32, '0')))
                     .build());
    pcs_ = std::make_shared<MockPeerCommunicationService>();
    EXPECT_CALL(*pcs_, on_commit())
        .WillRepeatedly(Return(commit_subject_.get_observable()));
    gate_transport = std::make_shared<OrderingGateTransportGrpc>(address);
    gate = std::make_shared<OrderingGateImpl>(gate_transport);
    gate->setPcs(*pcs_);
    gate_transport->subscribe(gate);

    service_transport = std::make_shared<OrderingServiceTransportGrpc>();
    counter = 2;

    wsv = std::make_shared<MockPeerQuery>();
  }

  void SetUp() override {
    fake_persistent_state =
        std::make_shared<MockOrderingServicePersistentState>();
  }

  void start() {
    grpc::ServerBuilder builder;
    int port = 0;
    builder.AddListeningPort(address, grpc::InsecureServerCredentials(), &port);

    builder.RegisterService(gate_transport.get());

    builder.RegisterService(service_transport.get());

    server = builder.BuildAndStart();
    ASSERT_NE(port, 0);
    ASSERT_TRUE(server);
  }

  void TearDown() override {
    proposals.clear();
    server->Shutdown();
  }

  TestSubscriber<std::shared_ptr<shared_model::interface::Proposal>> init(
      size_t times) {
    auto wrapper = make_test_subscriber<CallExact>(gate->on_proposal(), times);
    wrapper.subscribe();
    gate->on_proposal().subscribe([this](auto proposal) {
      proposals.push_back(proposal);

      // emulate commit event after receiving the proposal to perform next
      // round inside the peer.
      std::shared_ptr<shared_model::interface::Block> block =
          std::make_shared<shared_model::proto::Block>(
              TestBlockBuilder().build());
      commit_subject_.get_subscriber().on_next(
          rxcpp::observable<>::just(block));
    });
    gate->on_proposal().subscribe([this](auto) {
      counter--;
      cv.notify_one();
    });
    return wrapper;
  }

  auto initOs(size_t max_proposal, size_t commit_delay) {
    return std::make_shared<OrderingServiceImpl>(wsv,
                                                 max_proposal,
                                                 commit_delay,
                                                 service_transport,
                                                 fake_persistent_state,
                                                 false);
  }

  /**
   * Send a stub transaction to OS
   * @param i - number of transaction
   */
  void send_transaction() {
    auto tx = std::make_shared<shared_model::proto::Transaction>(
        shared_model::proto::TransactionBuilder()
            .createdTime(iroha::time::now())
            .creatorAccountId("admin@ru")
            .addAssetQuantity("admin@tu", "coin#coin", "1.0")
            .build()
            .signAndAddSignature(
                shared_model::crypto::DefaultCryptoAlgorithmType::
                    generateKeypair()));
    gate->propagateTransaction(tx);
  }

  std::string address{"0.0.0.0:50051"};
  std::shared_ptr<OrderingGateImpl> gate;
  std::shared_ptr<OrderingServiceImpl> service;

  /// Peer Communication Service and commit subject are required to emulate
  /// commits for Ordering Service
  std::shared_ptr<MockPeerCommunicationService> pcs_;
  rxcpp::subjects::subject<Commit> commit_subject_;

  std::vector<std::shared_ptr<shared_model::interface::Proposal>> proposals;
  std::atomic<size_t> counter;
  std::condition_variable cv;
  std::mutex m;
  std::shared_ptr<grpc::Server> server;

  std::shared_ptr<shared_model::interface::Peer> peer;
  std::shared_ptr<OrderingGateTransportGrpc> gate_transport;
  std::shared_ptr<OrderingServiceTransportGrpc> service_transport;
  std::shared_ptr<MockOrderingServicePersistentState> fake_persistent_state;
  std::shared_ptr<MockPeerQuery> wsv;
};

/**
 * @given Ordering service
 * @when  Send 8 transactions
 *        AND 2 transactions to OS
 * @then  Received proposal with 8 transactions
 *        AND proposal with 2 transactions
 */
TEST_F(OrderingGateServiceTest, SplittingBunchTransactions) {
  // 8 transaction -> proposal -> 2 transaction -> proposal
  EXPECT_CALL(*wsv, getLedgerPeers())
      .WillRepeatedly(Return(std::vector<wPeer>{peer}));

  const size_t max_proposal = 100;
  const size_t commit_delay = 400;

  EXPECT_CALL(*fake_persistent_state, loadProposalHeight())
      .Times(1)
      .WillOnce(Return(boost::optional<size_t>(2)));

  EXPECT_CALL(*fake_persistent_state, saveProposalHeight(3))
      .Times(1)
      .WillOnce(Return(true));
  EXPECT_CALL(*fake_persistent_state, saveProposalHeight(4))
      .Times(1)
      .WillOnce(Return(true));

  service = initOs(max_proposal, commit_delay);
  service_transport->subscribe(service);

  start();
  std::unique_lock<std::mutex> lk(m);
  auto wrapper = init(2);

  for (size_t i = 0; i < 8; ++i) {
    send_transaction();
  }

  cv.wait_for(lk, 10s);
  send_transaction();
  send_transaction();
  cv.wait_for(lk, 10s);

  std::this_thread::sleep_for(1s);
  ASSERT_EQ(proposals.size(), 2);
  ASSERT_EQ(proposals.at(0)->transactions().size(), 8);
  ASSERT_EQ(proposals.at(1)->transactions().size(), 2);
  ASSERT_EQ(counter, 0);
  ASSERT_TRUE(wrapper.validate());
}

/**
 * @given ordering service
 * @when a bunch of transaction has arrived
 * @then split transactions on to two proposal
 */
TEST_F(OrderingGateServiceTest, ProposalsReceivedWhenProposalSize) {
  // commits on the fulfilling proposal queue
  // 10 transaction -> proposal with 5 -> proposal with 5
  EXPECT_CALL(*wsv, getLedgerPeers())
      .WillRepeatedly(Return(std::vector<wPeer>{peer}));

  const size_t max_proposal = 5;
  const size_t commit_delay = 10000;

  EXPECT_CALL(*fake_persistent_state, loadProposalHeight())
      .Times(1)
      .WillOnce(Return(boost::optional<size_t>(2)));

  EXPECT_CALL(*fake_persistent_state, saveProposalHeight(3))
      .Times(1)
      .WillOnce(Return(true));
  EXPECT_CALL(*fake_persistent_state, saveProposalHeight(4))
      .Times(1)
      .WillOnce(Return(true));

  service = initOs(max_proposal, commit_delay);
  service_transport->subscribe(service);

  start();
  std::unique_lock<std::mutex> lk(m);
  auto wrapper = init(2);

  for (size_t j = 0; j < 2; ++j) {
    for (size_t i = 0; i < 5; ++i) {
      send_transaction();
    }
    // long == something wrong
    cv.wait_for(lk, 10s, [&] { return counter == (1 - j); });
  }

  ASSERT_TRUE(wrapper.validate()) << wrapper.what();
  ASSERT_EQ(proposals.size(), 2);
  ASSERT_EQ(counter, 0);

  for (auto &&proposal : proposals) {
    ASSERT_EQ(proposal->transactions().size(), 5);
  }
}

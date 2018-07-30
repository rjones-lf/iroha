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

#include "backend/protobuf/proto_proposal_factory.hpp"
#include "builders/protobuf/common_objects/proto_peer_builder.hpp"
#include "builders/protobuf/transaction.hpp"
#include "framework/test_subscriber.hpp"
#include "module/irohad/ametsuchi/ametsuchi_mocks.hpp"
#include "module/irohad/network/network_mocks.hpp"
#include "module/irohad/ordering/mock_ordering_service_persistent_state.hpp"
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

class OrderingGateServiceTest : public ::testing::Test {
 public:
  OrderingGateServiceTest() {
    factory_ = std::make_unique<shared_model::proto::ProtoProposalFactory<
        shared_model::validation::DefaultProposalValidator>>();
    pcs_ = std::make_shared<MockPeerCommunicationService>();
    EXPECT_CALL(*pcs_, on_commit())
        .WillRepeatedly(Return(commit_subject_.get_observable()));

    service_transport = std::make_shared<OrderingServiceTransportGrpc>();

    wsv = std::make_shared<MockPeerQuery>();
  }

  void SetUp() override {
    fake_persistent_state =
        std::make_shared<MockOrderingServicePersistentState>();
  }

  void initOs(size_t max_proposal) {
    service =
        std::make_shared<OrderingServiceImpl>(wsv,
                                              max_proposal,
                                              proposal_timeout.get_observable(),
                                              service_transport,
                                              fake_persistent_state,
                                              std::move(factory_));
    service_transport->subscribe(service);
  }

  void initGate(std::string address) {
    gate_transport = std::make_shared<OrderingGateTransportGrpc>(address);
    gate = std::make_shared<OrderingGateImpl>(gate_transport, 1, false);
    gate->setPcs(*pcs_);
    gate_transport->subscribe(gate);
  }

  void start(size_t max_proposal) {
    // make service server
    initOs(max_proposal);
    grpc::ServerBuilder service_builder;
    int service_port = 0;
    service_builder.AddListeningPort(
        kAddress + ":0", grpc::InsecureServerCredentials(), &service_port);
    service_builder.RegisterService(service_transport.get());
    service_server = service_builder.BuildAndStart();
    // check startup
    ASSERT_NE(service_port, 0);
    ASSERT_TRUE(service_server);

    // make gate server
    initGate(kAddress + ":" + std::to_string(service_port));
    grpc::ServerBuilder gate_builder;
    int gate_port = 0;
    gate_builder.AddListeningPort(
        kAddress + ":0", grpc::InsecureServerCredentials(), &gate_port);
    gate_builder.RegisterService(gate_transport.get());
    gate_server = gate_builder.BuildAndStart();
    // check startup
    ASSERT_NE(gate_port, 0);
    ASSERT_TRUE(gate_server);

    // setup peer
    peer = clone(
        shared_model::proto::PeerBuilder()
            .address(kAddress + ":" + std::to_string(gate_port))
            .pubkey(shared_model::interface::types::PubkeyType(
                std::string(shared_model::crypto::DefaultCryptoAlgorithmType::
                                kPublicKeyLength,
                            '0')))
            .build());
  }

  void TearDown() override {
    proposals.clear();
    gate_server->Shutdown();
    service_server->Shutdown();
  }

  TestSubscriber<std::shared_ptr<shared_model::interface::Proposal>> init(
      size_t times) {
    auto wrapper = make_test_subscriber<CallExact>(gate->on_proposal(), times);
    gate->on_proposal().subscribe([this](auto proposal) {
      proposals.push_back(proposal);
      // emulate commit event after receiving the proposal to perform next
      // round inside the peer.
      std::shared_ptr<shared_model::interface::Block> block =
          std::make_shared<shared_model::proto::Block>(
              TestBlockBuilder().height(proposal->height()).build());
      commit_subject_.get_subscriber().on_next(
          rxcpp::observable<>::just(block));
      counter--;
      cv.notify_one();
    });
    wrapper.subscribe();
    return wrapper;
  }

  /**
   * Send a stub transaction to OS
   * @param i - number of transaction
   */
  void send_transaction(size_t i) {
    auto tx = std::make_shared<shared_model::proto::Transaction>(
        shared_model::proto::TransactionBuilder()
            .createdTime(iroha::time::now())
            .creatorAccountId("admin@ru")
            .addAssetQuantity("coin#coin", "1.0")
            .quorum(1)
            .build()
            .signAndAddSignature(
                shared_model::crypto::DefaultCryptoAlgorithmType::
                    generateKeypair())
            .finish());
    gate->propagateTransaction(tx);
    // otherwise tx may come unordered
    std::this_thread::sleep_for(20ms);
  }

  void makeProposalTimeout() {
    proposal_timeout.get_subscriber().on_next(0);
  }

  void waitForGate() {
    std::unique_lock<std::mutex> lk(m);
    cv.wait_for(lk, 10s, [this] { return counter == 0; });
  }

  std::vector<std::shared_ptr<shared_model::interface::Proposal>> proposals;
  std::atomic<size_t> counter;
  std::shared_ptr<shared_model::interface::Peer> peer;
  std::shared_ptr<MockOrderingServicePersistentState> fake_persistent_state;
  std::shared_ptr<MockPeerQuery> wsv;

 private:
  std::shared_ptr<OrderingGateImpl> gate;
  std::shared_ptr<OrderingServiceImpl> service;

  /// Peer Communication Service and commit subject are required to emulate
  /// commits for Ordering Service
  std::shared_ptr<MockPeerCommunicationService> pcs_;
  rxcpp::subjects::subject<Commit> commit_subject_;
  rxcpp::subjects::subject<OrderingServiceImpl::TimeoutType> proposal_timeout;

  std::condition_variable cv;
  std::mutex m;
  std::shared_ptr<grpc::Server> service_server;
  std::shared_ptr<grpc::Server> gate_server;

  std::shared_ptr<OrderingGateTransportGrpc> gate_transport;
  std::shared_ptr<OrderingServiceTransportGrpc> service_transport;

  std::unique_ptr<shared_model::interface::ProposalFactory> factory_;

  const std::string kAddress = "127.0.0.1";
};

/**
 * @given Ordering Service
 * @when  Send 8 transactions
 *        AND 2 transactions to OS
 * @then  Received proposal with 8 transactions
 *        AND proposal with 2 transactions
 */
TEST_F(OrderingGateServiceTest, DISABLED_SplittingBunchTransactions) {
  const size_t max_proposal = 100;

  EXPECT_CALL(*fake_persistent_state, loadProposalHeight())
      .Times(1)
      .WillOnce(Return(boost::optional<size_t>(2)));
  EXPECT_CALL(*fake_persistent_state, saveProposalHeight(3))
      .Times(1)
      .WillOnce(Return(true));
  EXPECT_CALL(*fake_persistent_state, saveProposalHeight(4))
      .Times(1)
      .WillOnce(Return(true));

  start(max_proposal);

  EXPECT_CALL(*wsv, getLedgerPeers())
      .WillRepeatedly(Return(std::vector<wPeer>{peer}));

  auto wrapper = init(2);
  counter = 2;

  for (size_t i = 0; i < 8; ++i) {
    send_transaction(i + 1);
  }

  makeProposalTimeout();
  send_transaction(9);
  send_transaction(10);
  makeProposalTimeout();

  waitForGate();
  ASSERT_EQ(proposals.size(), 2);
  ASSERT_EQ(proposals.at(0)->transactions().size(), 8);
  ASSERT_EQ(proposals.at(1)->transactions().size(), 2);
  ASSERT_TRUE(wrapper.validate());
}

/**
 * @given Ordering Service with max proposal 5
 * @when Two bunches of 5 tx has been sent
 * @then Transactions are splitted in two proposals by 5 tx each
 */
TEST_F(OrderingGateServiceTest, DISABLED_ProposalsReceivedWhenProposalSize) {
  const size_t max_proposal = 5;

  EXPECT_CALL(*fake_persistent_state, loadProposalHeight())
      .Times(1)
      .WillOnce(Return(boost::optional<size_t>(2)));
  EXPECT_CALL(*fake_persistent_state, saveProposalHeight(3))
      .Times(1)
      .WillOnce(Return(true));
  EXPECT_CALL(*fake_persistent_state, saveProposalHeight(4))
      .Times(1)
      .WillOnce(Return(true));

  start(max_proposal);

  EXPECT_CALL(*wsv, getLedgerPeers())
      .WillRepeatedly(Return(std::vector<wPeer>{peer}));

  auto wrapper = init(2);
  counter = 2;

  for (size_t i = 0; i < 10; ++i) {
    send_transaction(i + 1);
  }

  waitForGate();
  ASSERT_EQ(proposals.size(), 2);
  ASSERT_EQ(proposals.at(0)->transactions().size(), 5);
  ASSERT_EQ(proposals.at(1)->transactions().size(), 5);
  ASSERT_TRUE(wrapper.validate());
}

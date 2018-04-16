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
#include "logger/logger.hpp"
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

using testing::_;
using testing::Return;

using wPeer = std::shared_ptr<shared_model::interface::Peer>;

logger::Logger logger_ = logger::log("OrderingServiceImpl");

class OrderingServiceImplProxy : public OrderingServiceImpl {
 public:
  using OrderingServiceImpl::OrderingServiceImpl;

  void init() {
    tx_awaiter_ = tx_await_.get_observable();
  }

  void onTransaction(
      std::shared_ptr<shared_model::interface::Transaction> tx) override {
    OrderingServiceImpl::onTransaction(tx);
    logger_->info("tx_await_ notify");
    tx_await_.get_subscriber().on_next(0);
  }

  template <typename T>
  void waitTransaction(T t) {
    try {
      tx_awaiter_.take(1).as_blocking().count();
    } catch (std::exception &e) {
      logger_->warn("Exception at waitTransaction: {}", e.what());
    }
  }

 private:
  rxcpp::subjects::subject<int> tx_await_;
  rxcpp::observable<int> tx_awaiter_;
};

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
    proposal_counter = 0;

    wsv = std::make_shared<MockPeerQuery>();
  }

  void SetUp() override {
    fake_persistent_state =
        std::make_shared<MockOrderingServicePersistentState>();
  }

  void start() {
    std::condition_variable cv;
    thread = std::thread([&cv, this] {
      grpc::ServerBuilder builder;
      int port = 0;
      builder.AddListeningPort(
          address, grpc::InsecureServerCredentials(), &port);

      builder.RegisterService(gate_transport.get());

      builder.RegisterService(service_transport.get());

      server = builder.BuildAndStart();
      ASSERT_NE(port, 0);
      ASSERT_TRUE(server);
      cv.notify_one();
      server->Wait();
    });

    std::mutex mtx;
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait_for(lock, 1s);
  }

  void TearDown() override {
    proposals.clear();
    server->Shutdown();
    if (thread.joinable()) {
      thread.join();
    }
  }

  TestSubscriber<std::shared_ptr<shared_model::interface::Proposal>> init(
      std::shared_ptr<OrderingServiceImplProxy> service, size_t times) {
    service->init();
    service_transport->subscribe(service);

    start();

    auto wrapper = make_test_subscriber<CallExact>(gate->on_proposal(), times);
    gate->on_proposal().subscribe([this](auto proposal) {
      proposal_counter++;
      proposals.push_back(proposal);

      // emulate commit event after receiving the proposal to perform next
      // round inside the peer.
      std::shared_ptr<shared_model::interface::Block> block =
          std::make_shared<shared_model::proto::Block>(
              TestBlockBuilder().build());
      commit_subject_.get_subscriber().on_next(
          rxcpp::observable<>::just(block));
      logger_->info("on_proposal");
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
            .txCounter(i)
            .createdTime(iroha::time::now())
            .creatorAccountId("admin@ru")
            .addAssetQuantity("admin@tu", "coin#coin", "1.0")
            .build()
            .signAndAddSignature(
                shared_model::crypto::DefaultCryptoAlgorithmType::
                    generateKeypair()));
    // network may change tx order, so wait until recieving
    gate->propagateTransaction(tx);
    service->waitTransaction(1s);
    logger_->info("tx_awaiter_ passed");
  }

  void waitProposal(size_t proposal_num) {
    proposal_timeout.get_subscriber().on_next(0);
    std::mutex m;
    std::unique_lock<std::mutex> lock(m);
    cv.wait_for(lock, 1s, [this, proposal_num] {
      return proposal_counter == proposal_num;
    });
  }

  std::string address{"0.0.0.0:50051"};
  std::shared_ptr<OrderingGateImpl> gate;
  std::shared_ptr<OrderingServiceImplProxy> service;

  /// Peer Communication Service and commit subject are required to emulate
  /// commits for Ordering Service
  std::shared_ptr<MockPeerCommunicationService> pcs_;
  rxcpp::subjects::subject<Commit> commit_subject_;
  rxcpp::subjects::subject<OrderingServiceImplProxy::Rep> proposal_timeout;

  std::vector<std::shared_ptr<shared_model::interface::Proposal>> proposals;
  std::atomic<size_t> proposal_counter;
  std::condition_variable cv;
  std::thread thread;
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
  const size_t proposal_num = 2;

  EXPECT_CALL(*fake_persistent_state, loadProposalHeight())
      .Times(1)
      .WillOnce(Return(boost::optional<size_t>(2)));

  EXPECT_CALL(*fake_persistent_state, saveProposalHeight(3))
      .Times(1)
      .WillOnce(Return(true));
  EXPECT_CALL(*fake_persistent_state, saveProposalHeight(4))
      .Times(1)
      .WillOnce(Return(true));

  service = std::make_shared<OrderingServiceImplProxy>(
      wsv,
      max_proposal,
      proposal_timeout.get_observable(),
      service_transport,
      fake_persistent_state);
  auto wrapper = init(service, proposal_num);

  for (size_t i = 0; i < 8; ++i) {
    send_transaction(i + 1);
  }

  waitProposal(1);
  send_transaction(9);
  send_transaction(10);
  waitProposal(2);

  ASSERT_EQ(proposals.size(), proposal_num);
  ASSERT_EQ(proposals.at(0)->transactions().size(), 8);
  ASSERT_EQ(proposals.at(1)->transactions().size(), 2);
  ASSERT_TRUE(wrapper.validate());

  size_t i = 1;
  for (auto &&proposal : proposals) {
    for (auto &&tx : proposal->transactions()) {
      ASSERT_EQ(tx->transactionCounter(), i++);
    }
  }
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
  const size_t proposal_num = 2;

  EXPECT_CALL(*fake_persistent_state, loadProposalHeight())
      .Times(1)
      .WillOnce(Return(boost::optional<size_t>(2)));

  EXPECT_CALL(*fake_persistent_state, saveProposalHeight(3))
      .Times(1)
      .WillOnce(Return(true));
  EXPECT_CALL(*fake_persistent_state, saveProposalHeight(4))
      .Times(1)
      .WillOnce(Return(true));

  service = std::make_shared<OrderingServiceImplProxy>(
      wsv,
      max_proposal,
      proposal_timeout.get_observable(),
      service_transport,
      fake_persistent_state);
  auto wrapper = init(service, proposal_num);

  for (size_t i = 0; i < 10; ++i) {
    send_transaction(i + 1);
  }

  waitProposal(2);

  ASSERT_TRUE(wrapper.validate());
  ASSERT_EQ(proposals.size(), proposal_num);

  size_t i = 1;
  for (auto &&proposal : proposals) {
    ASSERT_EQ(proposal->transactions().size(), 5);
    for (auto &&tx : proposal->transactions()) {
      ASSERT_EQ(tx->transactionCounter(), i++);
    }
  }
}

/**
 * @given ordering service with max_proposal==1
 * @when a bunch of transaction has arrived
 * @then the number of proposals are the same as tx number
 */
TEST_F(OrderingGateServiceTest, BatchOfSingleTxProposal) {
  EXPECT_CALL(*wsv, getLedgerPeers())
      .WillRepeatedly(Return(std::vector<wPeer>{peer}));

  const size_t max_proposal = 1;
  const size_t times = 10;
  const size_t proposal_num = times / max_proposal;

  int height_cnt = 2;
  EXPECT_CALL(*fake_persistent_state, loadProposalHeight())
      .WillRepeatedly(Return(boost::optional<size_t>(height_cnt++)));

  EXPECT_CALL(*fake_persistent_state, saveProposalHeight(_))
      .WillRepeatedly(Return(true));

  service = std::make_shared<OrderingServiceImplProxy>(
      wsv,
      max_proposal,
      proposal_timeout.get_observable(),
      service_transport,
      fake_persistent_state);
  auto wrapper = init(service, proposal_num);

  for (size_t i = 0; i < times; ++i) {
    send_transaction(i + 1);
  }

  waitProposal(proposal_num);

  ASSERT_TRUE(wrapper.validate());
  ASSERT_EQ(proposals.size(), proposal_num);

  size_t i = 1;
  for (auto &&proposal : proposals) {
    ASSERT_EQ(proposal->transactions().size(), max_proposal);
    for (auto &&tx : proposal->transactions()) {
      ASSERT_EQ(tx->transactionCounter(), i++);
    }
  }
}

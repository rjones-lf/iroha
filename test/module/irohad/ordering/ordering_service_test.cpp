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

#include "backend/protobuf/common_objects/peer.hpp"
#include "builders/protobuf/common_objects/proto_peer_builder.hpp"
#include "builders/protobuf/transaction.hpp"
#include "logger/logger.hpp"
#include "module/irohad/ametsuchi/ametsuchi_mocks.hpp"
#include "module/irohad/network/network_mocks.hpp"
#include "module/irohad/ordering/mock_ordering_service_persistent_state.hpp"
#include "module/shared_model/builders/protobuf/test_proposal_builder.hpp"
#include "module/shared_model/builders/protobuf/test_transaction_builder.hpp"
#include "ordering/impl/ordering_service_impl.hpp"
#include "ordering/impl/ordering_service_transport_grpc.hpp"

using namespace iroha;
using namespace iroha::ordering;
using namespace iroha::network;
using namespace iroha::ametsuchi;
using namespace std::chrono_literals;

using ::testing::_;
using ::testing::AtLeast;
using ::testing::DoAll;
using ::testing::Invoke;
using ::testing::InvokeWithoutArgs;
using ::testing::Return;

static logger::Logger log_ = logger::testLog("OrderingServiceTest");

class MockOrderingServiceTransport : public network::OrderingServiceTransport {
 public:
  void subscribe(std::shared_ptr<network::OrderingServiceNotification>
                     subscriber) override {
    subscriber_ = subscriber;
  }

  void publishProposal(
      std::unique_ptr<shared_model::interface::Proposal> proposal,
      const std::vector<std::string> &peers) override {
    return publishProposalProxy(proposal.get(), peers);
  }

  MOCK_METHOD2(publishProposalProxy,
               void(shared_model::interface::Proposal *proposal,
                    const std::vector<std::string> &peers));

  std::weak_ptr<network::OrderingServiceNotification> subscriber_;
};

class OrderingServiceTest : public ::testing::Test {
 public:
  OrderingServiceTest() {
    peer = clone(shared_model::proto::PeerBuilder()
                     .address(address)
                     .pubkey(shared_model::interface::types::PubkeyType(
                         std::string(32, '0')))
                     .build());
  }

  void SetUp() override {
    wsv = std::make_shared<MockPeerQuery>();
    fake_transport = std::make_shared<MockOrderingServiceTransport>();
    fake_persistent_state =
        std::make_shared<MockOrderingServicePersistentState>();
  }

  auto createTx() {
    return std::make_shared<shared_model::proto::Transaction>(
        shared_model::proto::TransactionBuilder()
            .txCounter(2)
            .createdTime(iroha::time::now())
            .creatorAccountId("admin@ru")
            .addAssetQuantity("admin@tu", "coin#coin", "1.0")
            .build()
            .signAndAddSignature(
                shared_model::crypto::DefaultCryptoAlgorithmType::
                    generateKeypair()));
  }

  void timeoutTick() {
    log_->info("proposal timeout tick");
    proposal_timeout.get_subscriber().on_next(0);
  }

  std::shared_ptr<MockOrderingServiceTransport> fake_transport;
  std::shared_ptr<MockOrderingServicePersistentState> fake_persistent_state;
  std::string address{"0.0.0.0:50051"};
  std::shared_ptr<shared_model::interface::Peer> peer;
  std::shared_ptr<MockPeerQuery> wsv;
  rxcpp::subjects::subject<OrderingServiceImpl::Rep> proposal_timeout;
};

/**
 * @given OrderingService and MockOrderingServiceTransport
 * @when publishProposal is called at transport
 * @then publishProposalProxy is called
 */
TEST_F(OrderingServiceTest, PublishProxy) {
  const size_t max_proposal = 5;

  EXPECT_CALL(*fake_persistent_state, loadProposalHeight())
      .Times(1)
      .WillOnce(Return(boost::optional<size_t>(2)));

  auto ordering_service =
      std::make_shared<OrderingServiceImpl>(wsv,
                                            max_proposal,
                                            proposal_timeout.get_observable(),
                                            fake_transport,
                                            fake_persistent_state);
  fake_transport->subscribe(ordering_service);

  EXPECT_CALL(*fake_transport, publishProposalProxy(_, _)).Times(1);

  fake_transport->publishProposal(
      std::make_unique<shared_model::proto::Proposal>(
          TestProposalBuilder()
              .height(1)
              .createdTime(iroha::time::now())
              .build()),
      {});
}

/**
 * @given OrderingService with max_proposal==5 and only self peer
 *        and MockOrderingServiceTransport
 *        and MockOrderingServicePersistentState
 * @when OrderingService::onTransaction called 10 times
 * @then publishProposalProxy called twice
 *       and proposal height was loaded once and saved twice
 */
TEST_F(OrderingServiceTest, ValidWhenProposalSizeStrategy) {
  const size_t max_proposal = 5;
  const size_t tx_num = 10;

  EXPECT_CALL(*fake_persistent_state, saveProposalHeight(_))
      .Times(2)
      .WillRepeatedly(Return(true));
  EXPECT_CALL(*fake_persistent_state, loadProposalHeight())
      .Times(1)
      .WillOnce(Return(boost::optional<size_t>(2)));
  EXPECT_CALL(*wsv, getLedgerPeers())
      .WillRepeatedly(Return(std::vector<decltype(peer)>{peer}));

  auto ordering_service =
      std::make_shared<OrderingServiceImpl>(wsv,
                                            max_proposal,
                                            proposal_timeout.get_observable(),
                                            fake_transport,
                                            fake_persistent_state);
  fake_transport->subscribe(ordering_service);

  EXPECT_CALL(*fake_transport, publishProposalProxy(_, _))
      .Times(tx_num / max_proposal);

  for (size_t i = 0; i < tx_num; ++i) {
    ordering_service->onTransaction(createTx());
  }
}

/**
 * @given OrderingService with big enough max_proposal and only self peer
 *        and MockOrderingServiceTransport
 *        and MockOrderingServicePersistentState
 * @when OrderingService::onTransaction called 8 times
 *       and after triggered timeout
 *       and then repeat with 2 onTransaction calls
 * @then publishProposalProxy called twice
 *       and proposal height was loaded once and saved twice
 */
TEST_F(OrderingServiceTest, ValidWhenTimerStrategy) {
  const size_t max_proposal = 100;

  EXPECT_CALL(*fake_persistent_state, saveProposalHeight(_))
      .Times(2)
      .WillRepeatedly(Return(true));
  EXPECT_CALL(*fake_persistent_state, loadProposalHeight())
      .Times(1)
      .WillOnce(Return(boost::optional<size_t>(2)));
  EXPECT_CALL(*wsv, getLedgerPeers())
      .WillRepeatedly(Return(std::vector<decltype(peer)>{peer}));

  auto ordering_service =
      std::make_shared<OrderingServiceImpl>(wsv,
                                            max_proposal,
                                            proposal_timeout.get_observable(),
                                            fake_transport,
                                            fake_persistent_state);
  fake_transport->subscribe(ordering_service);

  EXPECT_CALL(*fake_transport, publishProposalProxy(_, _))
      .Times(2)
      .WillRepeatedly(
          InvokeWithoutArgs([&] { log_->info("Proposal send to grpc"); }));

  for (size_t i = 0; i < 8; ++i) {
    ordering_service->onTransaction(createTx());
  }

  timeoutTick();
  ordering_service->onTransaction(createTx());
  ordering_service->onTransaction(createTx());
  timeoutTick();
}

/**
 * @given Ordering service and the persistent state that cannot save
 proposals
 * @when onTransaction is called
 * @then no published proposal
 */
TEST_F(OrderingServiceTest, BrokenPersistentState) {
  const size_t max_proposal = 1;
  EXPECT_CALL(*fake_persistent_state, loadProposalHeight())
      .Times(1)
      .WillOnce(Return(boost::optional<size_t>(1)));
  EXPECT_CALL(*fake_persistent_state, saveProposalHeight(2))
      .Times(1)
      .WillRepeatedly(Return(false));

  auto ordering_service =
      std::make_shared<OrderingServiceImpl>(wsv,
                                            max_proposal,
                                            proposal_timeout.get_observable(),
                                            fake_transport,
                                            fake_persistent_state);
  ordering_service->onTransaction(createTx());
  timeoutTick();
}

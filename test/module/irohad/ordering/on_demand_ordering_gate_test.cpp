/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ordering/impl/on_demand_ordering_gate.hpp"

#include <gtest/gtest.h>
#include <boost/range/adaptor/indirected.hpp>
#include "framework/test_subscriber.hpp"
#include "interfaces/iroha_internal/transaction_batch_impl.hpp"
#include "module/irohad/ametsuchi/ametsuchi_mocks.hpp"
#include "module/irohad/ordering/ordering_mocks.hpp"
#include "module/shared_model/interface_mocks.hpp"

using namespace iroha;
using namespace iroha::ordering;
using namespace iroha::ordering::transport;
using namespace iroha::network;
using namespace framework::test_subscriber;

using ::testing::_;
using ::testing::ByMove;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::Truly;
using ::testing::UnorderedElementsAre;
using ::testing::UnorderedElementsAreArray;

class OnDemandOrderingGateTest : public ::testing::Test {
 public:
  void SetUp() override {
    ordering_service = std::make_shared<MockOnDemandOrderingService>();
    notification = std::make_shared<MockOdOsNotification>();
    cache = std::make_shared<cache::MockOrderingGateCache>();
    auto ufactory = std::make_unique<NiceMock<MockUnsafeProposalFactory>>();
    factory = ufactory.get();
    tx_cache = std::make_shared<ametsuchi::MockTxPresenceCache>();
    ON_CALL(*tx_cache,
            check(testing::Matcher<const shared_model::crypto::Hash &>(_)))
        .WillByDefault(
            Return(boost::make_optional<ametsuchi::TxCacheStatusType>(
                iroha::ametsuchi::tx_cache_status_responses::Missing())));
    ordering_gate =
        std::make_shared<OnDemandOrderingGate>(ordering_service,
                                               notification,
                                               rounds.get_observable(),
                                               cache,
                                               std::move(ufactory),
                                               tx_cache,
                                               initial_round);
  }

  rxcpp::subjects::subject<OnDemandOrderingGate::BlockRoundEventType> rounds;
  std::shared_ptr<MockOnDemandOrderingService> ordering_service;
  std::shared_ptr<MockOdOsNotification> notification;
  MockUnsafeProposalFactory *factory;
  std::shared_ptr<ametsuchi::MockTxPresenceCache> tx_cache;
  std::shared_ptr<OnDemandOrderingGate> ordering_gate;

  std::shared_ptr<cache::MockOrderingGateCache> cache;

  const consensus::Round initial_round = {2, 1};
};

/**
 * @given initialized ordering gate
 * @when a batch is received
 * @then it is passed to the ordering service
 */
TEST_F(OnDemandOrderingGateTest, propagateBatch) {
  auto hash1 = shared_model::interface::types::HashType("");
  auto batch = createMockBatchWithHash(hash1);
  OdOsNotification::CollectionType collection{batch};

  EXPECT_CALL(*cache, addToBack(UnorderedElementsAre(batch))).Times(1);
  EXPECT_CALL(*notification, onBatches(initial_round, collection)).Times(1);

  ordering_gate->propagateBatch(batch);
}

/**
 * @given initialized ordering gate
 * @when a block round event with height is received from the PCS
 * AND a proposal is successfully retrieved from the network
 * @then new proposal round based on the received height is initiated
 */
TEST_F(OnDemandOrderingGateTest, BlockEvent) {
  consensus::Round round{3, 1};

  boost::optional<OdOsNotification::ProposalType> oproposal(
      std::make_unique<MockProposal>());
  auto proposal = oproposal.value().get();

  EXPECT_CALL(*ordering_service, onCollaborationOutcome(round)).Times(1);
  EXPECT_CALL(*notification, onRequestProposal(round))
      .WillOnce(Return(ByMove(std::move(oproposal))));
  EXPECT_CALL(*factory, unsafeCreateProposal(_, _, _)).Times(0);

  auto gate_wrapper =
      make_test_subscriber<CallExact>(ordering_gate->on_proposal(), 1);
  gate_wrapper.subscribe([&](auto val) { ASSERT_EQ(val.get(), proposal); });

  rounds.get_subscriber().on_next(OnDemandOrderingGate::BlockEvent{round, {}});

  ASSERT_TRUE(gate_wrapper.validate());
}

/**
 * @given initialized ordering gate
 * @when an empty block round event is received from the PCS
 * AND a proposal is successfully retrieved from the network
 * @then new proposal round based on the received height is initiated
 */
TEST_F(OnDemandOrderingGateTest, EmptyEvent) {
  consensus::Round round{initial_round.block_round,
                         initial_round.reject_round + 1};

  boost::optional<OdOsNotification::ProposalType> oproposal(
      std::make_unique<MockProposal>());
  auto proposal = oproposal.value().get();

  EXPECT_CALL(*ordering_service, onCollaborationOutcome(round)).Times(1);
  EXPECT_CALL(*notification, onRequestProposal(round))
      .WillOnce(Return(ByMove(std::move(oproposal))));
  EXPECT_CALL(*factory, unsafeCreateProposal(_, _, _)).Times(0);

  auto gate_wrapper =
      make_test_subscriber<CallExact>(ordering_gate->on_proposal(), 1);
  gate_wrapper.subscribe([&](auto val) { ASSERT_EQ(val.get(), proposal); });

  rounds.get_subscriber().on_next(OnDemandOrderingGate::EmptyEvent{});

  ASSERT_TRUE(gate_wrapper.validate());
}

/**
 * @given initialized ordering gate
 * @when a block round event with height is received from the PCS
 * AND a proposal is not retrieved from the network
 * @then new empty proposal round based on the received height is initiated
 */
TEST_F(OnDemandOrderingGateTest, BlockEventNoProposal) {
  consensus::Round round{3, 1};

  boost::optional<OdOsNotification::ProposalType> oproposal;

  EXPECT_CALL(*ordering_service, onCollaborationOutcome(round)).Times(1);
  EXPECT_CALL(*notification, onRequestProposal(round))
      .WillOnce(Return(ByMove(std::move(oproposal))));

  OdOsNotification::ProposalType uproposal;
  auto proposal = uproposal.get();

  EXPECT_CALL(*factory, unsafeCreateProposal(_, _, _))
      .WillOnce(Return(ByMove(std::move(uproposal))));

  auto gate_wrapper =
      make_test_subscriber<CallExact>(ordering_gate->on_proposal(), 1);
  gate_wrapper.subscribe([&](auto val) { ASSERT_EQ(val.get(), proposal); });

  rounds.get_subscriber().on_next(OnDemandOrderingGate::BlockEvent{round, {}});

  ASSERT_TRUE(gate_wrapper.validate());
}

/**
 * @given initialized ordering gate
 * @when an empty block round event is received from the PCS
 * AND a proposal is not retrieved from the network
 * @then new empty proposal round based on the received height is initiated
 */
TEST_F(OnDemandOrderingGateTest, EmptyEventNoProposal) {
  consensus::Round round{initial_round.block_round,
                         initial_round.reject_round + 1};

  boost::optional<OdOsNotification::ProposalType> oproposal;

  EXPECT_CALL(*ordering_service, onCollaborationOutcome(round)).Times(1);
  EXPECT_CALL(*notification, onRequestProposal(round))
      .WillOnce(Return(ByMove(std::move(oproposal))));

  OdOsNotification::ProposalType uproposal;
  auto proposal = uproposal.get();

  EXPECT_CALL(*factory, unsafeCreateProposal(_, _, _))
      .WillOnce(Return(ByMove(std::move(uproposal))));

  auto gate_wrapper =
      make_test_subscriber<CallExact>(ordering_gate->on_proposal(), 1);
  gate_wrapper.subscribe([&](auto val) { ASSERT_EQ(val.get(), proposal); });

  rounds.get_subscriber().on_next(OnDemandOrderingGate::EmptyEvent{});

  ASSERT_TRUE(gate_wrapper.validate());
}

/**
 * @given initialized ordering gate
 * @when new proposal arrives and the transaction was already committed
 * @then the resulting proposal emitted by ordering gate does not contain
 * this transaction
 */
TEST_F(OnDemandOrderingGateTest, ReplayedTransactionInProposal) {
  OnDemandOrderingGate::BlockEvent event = {initial_round, {}};

  // initialize mock transaction
  auto tx1 = std::make_shared<NiceMock<MockTransaction>>();
  auto hash = shared_model::crypto::Hash("mock code is readable");
  ON_CALL(*tx1, hash()).WillByDefault(testing::ReturnRef(testing::Const(hash)));
  std::vector<decltype(tx1)> txs{tx1};
  auto tx_range = txs | boost::adaptors::indirected;

  // initialize mock proposal
  auto proposal = std::make_unique<NiceMock<MockProposal>>();
  ON_CALL(*proposal, transactions()).WillByDefault(Return(tx_range));
  boost::optional<OdOsNotification::ProposalType> arriving_proposal =
      std::unique_ptr<shared_model::interface::Proposal>(std::move(proposal));

  // set expectations for ordering service
  EXPECT_CALL(*ordering_service, onCollaborationOutcome(initial_round))
      .Times(1);
  EXPECT_CALL(*notification, onRequestProposal(initial_round))
      .WillOnce(Return(ByMove(std::move(arriving_proposal))));
  EXPECT_CALL(*tx_cache,
              check(testing::Matcher<const shared_model::crypto::Hash &>(_)))
      .WillOnce(Return(boost::make_optional<ametsuchi::TxCacheStatusType>(
          iroha::ametsuchi::tx_cache_status_responses::Committed())));
  // expect proposal to be created without any transactions because it was
  // removed by tx cache
  EXPECT_CALL(
      *factory,
      unsafeCreateProposal(
          _, _, MockUnsafeProposalFactory::TransactionsCollectionType()))
      .Times(1);

  auto gate_wrapper =
      make_test_subscriber<CallExact>(ordering_gate->on_proposal(), 1);
  gate_wrapper.subscribe([&](auto proposal) {});
  rounds.get_subscriber().on_next(event);

  ASSERT_TRUE(gate_wrapper.validate());
}

/**
 * @given initialized ordering gate
 * @when block event with no batches is emitted @and cache contains batch1 and
 * batch2 on the head
 * @then batch1 and batch2 are propagated to network
 */
TEST_F(OnDemandOrderingGateTest, SendBatchesFromTheCache) {
  // prepare hashes for mock batches
  shared_model::interface::types::HashType hash1("hash1");
  shared_model::interface::types::HashType hash2("hash2");

  // prepare batches
  auto batch1 = createMockBatchWithHash(hash1);
  auto batch2 = createMockBatchWithHash(hash2);

  cache::OrderingGateCache::BatchesSetType collection{batch1, batch2};

  EXPECT_CALL(*cache, pop()).WillOnce(Return(collection));

  EXPECT_CALL(*cache, addToBack(UnorderedElementsAreArray(collection)))
      .Times(1);
  EXPECT_CALL(*notification,
              onBatches(initial_round, UnorderedElementsAreArray(collection)))
      .Times(1);

  rounds.get_subscriber().on_next(
      OnDemandOrderingGate::BlockEvent{initial_round, {}});
}

/**
 * @given initialized ordering gate
 * @when an block round event is received from the PCS
 * @then all batches from that event are removed from the cache
 */
TEST_F(OnDemandOrderingGateTest, BatchesRemoveFromCache) {
  // prepare hashes for mock batches
  shared_model::interface::types::HashType hash1("hash1");
  shared_model::interface::types::HashType hash2("hash2");

  // prepare batches
  auto batch1 = createMockBatchWithHash(hash1);
  auto batch2 = createMockBatchWithHash(hash2);

  EXPECT_CALL(*cache, pop()).Times(1);
  EXPECT_CALL(*cache, remove(UnorderedElementsAre(hash1, hash2))).Times(1);

  rounds.get_subscriber().on_next(
      OnDemandOrderingGate::BlockEvent{initial_round, {hash1, hash2}});
}

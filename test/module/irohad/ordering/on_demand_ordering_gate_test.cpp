/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ordering/impl/on_demand_ordering_gate.hpp"

#include <gtest/gtest.h>
#include "framework/batch_helper.hpp"
#include "framework/test_subscriber.hpp"
#include "interfaces/iroha_internal/transaction_batch_impl.hpp"
#include "module/irohad/ordering/ordering_mocks.hpp"
#include "module/shared_model/interface_mocks.hpp"
#include "ordering/impl/og_cache/on_demand_cache.hpp"

using namespace iroha::ordering;
using namespace iroha::ordering::transport;
using namespace iroha::network;
using namespace framework::test_subscriber;

using ::testing::_;
using ::testing::ByMove;
using ::testing::Return;
using ::testing::Truly;

struct OnDemandOrderingGateTest : public ::testing::Test {
  void SetUp() override {
    ordering_service = std::make_shared<MockOnDemandOrderingService>();
    notification = std::make_shared<MockOdOsNotification>();
    cache = std::make_shared<cache::OnDemandCache>();
    auto ufactory = std::make_unique<MockUnsafeProposalFactory>();
    factory = ufactory.get();
    ordering_gate =
        std::make_shared<OnDemandOrderingGate>(ordering_service,
                                               notification,
                                               rounds.get_observable(),
                                               cache,
                                               std::move(ufactory),
                                               initial_round);
  }

  std::shared_ptr<shared_model::interface::Block> createBlockWithHeight(size_t height) {
    auto block = std::make_shared<MockBlock>();
    EXPECT_CALL(*block, height()).WillOnce(Return(height));
    return block;
  }

  rxcpp::subjects::subject<OnDemandOrderingGate::BlockRoundEventType> rounds;
  std::shared_ptr<MockOnDemandOrderingService> ordering_service;
  std::shared_ptr<MockOdOsNotification> notification;
  MockUnsafeProposalFactory *factory;
  std::shared_ptr<OnDemandOrderingGate> ordering_gate;

  std::shared_ptr<cache::OnDemandCache> cache;

  const Round initial_round = {2, 1};
};

/**
 * @given initialized ordering gate
 * @when a batch is received
 * @then it is passed to the ordering service
 */
TEST_F(OnDemandOrderingGateTest, propagateBatch) {
  OdOsNotification::CollectionType collection;
  std::shared_ptr<shared_model::interface::TransactionBatch> batch =
      std::make_shared<shared_model::interface::TransactionBatchImpl>(
          collection);

  EXPECT_CALL(*notification, onTransactions(initial_round, collection))
      .Times(1);

  ordering_gate->propagateBatch(batch);
}

/**
 * @given initialized ordering gate
 * @when a block round event with height is received from the PCS
 * AND a proposal is successfully retrieved from the network
 * @then new proposal round based on the received height is initiated
 */
TEST_F(OnDemandOrderingGateTest, BlockEvent) {
  OnDemandOrderingGate::BlockEvent event{3};
  Round round{event.height, 1};

  boost::optional<OdOsNotification::ProposalType> oproposal(nullptr);
  auto proposal = oproposal.value().get();

  EXPECT_CALL(*ordering_service, onCollaborationOutcome(round)).Times(1);
  EXPECT_CALL(*notification, onRequestProposal(round))
      .WillOnce(Return(ByMove(std::move(oproposal))));
  EXPECT_CALL(*factory, unsafeCreateProposal(_, _, _)).Times(0);

  auto gate_wrapper =
      make_test_subscriber<CallExact>(ordering_gate->on_proposal(), 1);
  gate_wrapper.subscribe([&](auto val) { ASSERT_EQ(val.get(), proposal); });

  rounds.get_subscriber().on_next(event);

  ASSERT_TRUE(gate_wrapper.validate());
}

/**
 * @given initialized ordering gate
 * @when an empty block round event is received from the PCS
 * AND a proposal is successfully retrieved from the network
 * @then new proposal round based on the received height is initiated
 */
TEST_F(OnDemandOrderingGateTest, EmptyEvent) {
  Round round{initial_round.block_round, initial_round.reject_round + 1};

  boost::optional<OdOsNotification::ProposalType> oproposal(nullptr);
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
  OnDemandOrderingGate::BlockEvent event{3};
  Round round{event.height, 1};

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

  rounds.get_subscriber().on_next(event);

  ASSERT_TRUE(gate_wrapper.validate());
}

/**
 * @given initialized ordering gate
 * @when an empty block round event is received from the PCS
 * AND a proposal is not retrieved from the network
 * @then new empty proposal round based on the received height is initiated
 */
TEST_F(OnDemandOrderingGateTest, EmptyEventNoProposal) {
  Round round{initial_round.block_round, initial_round.reject_round + 1};

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

TEST_F(OnDemandOrderingGateTest, SendTransactions) {
  auto now = iroha::time::now();
  auto batch1 = std::shared_ptr<shared_model::interface::TransactionBatch>(
      framework::batch::createValidBatch(1, now + 1));
  auto batch2 = std::shared_ptr<shared_model::interface::TransactionBatch>(
      framework::batch::createValidBatch(1, now + 2));
  auto batch3 = std::shared_ptr<shared_model::interface::TransactionBatch>(
      framework::batch::createValidBatch(1, now + 3));

  //-------------------------------------------------------------------
  ordering_gate->propagateBatch(batch1);
  /**
   * 1. {}
   * 2. {}
   * 3. {batch1}
   */
  ASSERT_EQ(cache->front().size(), 0);
  ASSERT_EQ(cache->back().size(), 1);

  rounds.get_subscriber().on_next(createBlockWithHeight(1));
  /**
   * 1. {}
   * 2. {batch1}
   * 3. {}
   */
  ASSERT_EQ(cache->front().size(), 0);
  ASSERT_EQ(cache->back().size(), 0);

  //-------------------------------------------------------------------
  ordering_gate->propagateBatch(batch2);
  /**
   * 1. {}
   * 2. {batch1}
   * 3. {batch2}
   */
  ASSERT_EQ(cache->front().size(), 0);
  ASSERT_EQ(cache->back().size(), 1);

  rounds.get_subscriber().on_next(createBlockWithHeight(2));
  /**
   * 1. {batch1}
   * 2. {batch2}
   * 3. {}
   */
  ASSERT_EQ(cache->front().size(), 1);
  ASSERT_EQ(cache->back().size(), 0);

  //-------------------------------------------------------------------
  ordering_gate->propagateBatch(batch3);
  /**
   * 1. {}
   * 2. {batch2}
   * 3. {batch1, batch3}
   */
  ASSERT_EQ(cache->front().size(), 0);
  ASSERT_EQ(cache->back().size(), 2);

  rounds.get_subscriber().on_next(createBlockWithHeight(3));
  /**
   * 1. {batch2}
   * 2. {batch1, batch3}
   * 3. {}
   */
  ASSERT_EQ(cache->front().size(), 1);
  ASSERT_EQ(cache->back().size(), 0);
}

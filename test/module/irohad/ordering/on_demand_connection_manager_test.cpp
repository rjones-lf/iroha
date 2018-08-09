/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ordering/impl/on_demand_connection_manager.hpp"

#include <gtest/gtest.h>
#include "interfaces/iroha_internal/proposal.hpp"
#include "module/irohad/ordering/ordering_mocks.hpp"
#include "module/shared_model/interface_mocks.hpp"

using namespace iroha::ordering;
using namespace iroha::ordering::transport;

using ::testing::ByMove;
using ::testing::Ref;
using ::testing::Return;

/**
 * Create unique_ptr with MockOdOsNotification, save to var, and return it
 */
ACTION_P(CreateAndSave, var) {
  auto result = std::make_unique<MockOdOsNotification>();
  *var = result.get();
  return std::unique_ptr<OdOsNotification>(std::move(result));
}

struct OnDemandConnectionManagerTest : public ::testing::Test {
  void SetUp() override {
    factory = std::make_shared<MockOdOsNotificationFactory>();
    manager = std::make_shared<OnDemandConnectionManager>(
        factory, peers.get_observable());

    auto set = [this](auto &field, auto &ptr) {
      field = std::make_shared<MockPeer>();

      EXPECT_CALL(*factory, create(Ref(*field)))
          .WillRepeatedly(CreateAndSave(&ptr));
    };

    set(cpeers.issuer, issuer);
    set(cpeers.current_consumer, current_consumer);
    set(cpeers.previous_consumer, previous_consumer);

    peers.get_subscriber().on_next(cpeers);
  }

  OnDemandConnectionManager::CurrentPeers cpeers;
  MockOdOsNotification *issuer, *previous_consumer, *current_consumer;

  rxcpp::subjects::subject<OnDemandConnectionManager::CurrentPeers> peers;
  std::shared_ptr<MockOdOsNotificationFactory> factory;
  std::shared_ptr<OnDemandConnectionManager> manager;
};

/**
 * @given OnDemandConnectionManager
 * @when peers observable is triggered
 * @then new peers are requested from factory
 */
TEST_F(OnDemandConnectionManagerTest, FactoryUsed) {
  ASSERT_NE(issuer, nullptr);
  ASSERT_NE(previous_consumer, nullptr);
  ASSERT_NE(current_consumer, nullptr);
}

/**
 * @given initialized OnDemandConnectionManager
 * @when onTransactions is called
 * @then peers get data for propagation
 */
TEST_F(OnDemandConnectionManagerTest, onTransactions) {
  OdOsNotification::CollectionType collection;
  EXPECT_CALL(*previous_consumer, onTransactions(collection)).Times(1);
  EXPECT_CALL(*current_consumer, onTransactions(collection)).Times(1);

  manager->onTransactions(collection);
}

/**
 * @given initialized OnDemandConnectionManager
 * @when onRequestProposal is called
 * AND proposal is returned
 * @then peer is triggered
 * AND return data is forwarded
 */
TEST_F(OnDemandConnectionManagerTest, onRequestProposal) {
  RoundType round;
  boost::optional<OnDemandConnectionManager::ProposalType> oproposal =
      OnDemandConnectionManager::ProposalType{};
  auto proposal = oproposal.value().get();
  EXPECT_CALL(*issuer, onRequestProposal(round))
      .WillOnce(Return(ByMove(std::move(oproposal))));

  auto result = manager->onRequestProposal(round);

  ASSERT_TRUE(result);
  ASSERT_EQ(result.value().get(), proposal);
}

/**
 * @given initialized OnDemandConnectionManager
 * @when onRequestProposal is called
 * AND no proposal is returned
 * @then peer is triggered
 * AND return data is forwarded
 */
TEST_F(OnDemandConnectionManagerTest, onRequestProposalNone) {
  RoundType round;
  boost::optional<OnDemandConnectionManager::ProposalType> oproposal;
  EXPECT_CALL(*issuer, onRequestProposal(round))
      .WillOnce(Return(ByMove(std::move(oproposal))));

  auto result = manager->onRequestProposal(round);

  ASSERT_FALSE(result);
}

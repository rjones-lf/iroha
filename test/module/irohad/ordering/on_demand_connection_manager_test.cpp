/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ordering/impl/on_demand_connection_manager.hpp"

#include <gtest/gtest.h>
#include <boost/range/combine.hpp>
#include "interfaces/iroha_internal/proposal.hpp"
#include "module/irohad/ordering/ordering_mocks.hpp"
#include "module/shared_model/interface_mocks.hpp"
#include "ordering/impl/on_demand_common.hpp"

using namespace iroha;
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

    auto set = [this](auto &field, auto &ptr) {
      field = std::make_shared<MockPeer>();

      EXPECT_CALL(*factory, create(Ref(*field)))
          .WillRepeatedly(CreateAndSave(&ptr));
    };

    for (auto &&pair : boost::combine(cpeers.peers, connections)) {
      set(boost::get<0>(pair), boost::get<1>(pair));
    }

    manager = std::make_shared<OnDemandConnectionManager>(
        factory, peers.get_observable(), cpeers);
  }

  OnDemandConnectionManager::CurrentPeers cpeers;
  OnDemandConnectionManager::PeerCollectionType<MockOdOsNotification *>
      connections;

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
  for (auto &peer : connections) {
    ASSERT_NE(peer, nullptr);
  }
}

/**
 * @given initialized OnDemandConnectionManager
 * @when onBatches is called
 * @then peers get data for propagation
 */
TEST_F(OnDemandConnectionManagerTest, onBatches) {
  OdOsNotification::CollectionType collection;
  consensus::Round round{1, 2};

  auto set_expect = [&](OnDemandConnectionManager::PeerType type,
                        consensus::Round round) {
    EXPECT_CALL(*connections[type], onBatches(round, collection)).Times(1);
  };

  set_expect(
      OnDemandConnectionManager::kCurrentRoundRejectConsumer,
      {round.block_round, currentRejectRoundConsumer(round.reject_round)});
  set_expect(OnDemandConnectionManager::kNextRoundRejectConsumer,
             {round.block_round + 1, kNextRejectRoundConsumer});
  set_expect(OnDemandConnectionManager::kNextRoundCommitConsumer,
             {round.block_round + 2, kNextCommitRoundConsumer});

  manager->onBatches(round, collection);
}

/**
 * @given initialized OnDemandConnectionManager
 * @when onRequestProposal is called
 * AND proposal is returned
 * @then peer is triggered
 * AND return data is forwarded
 */
TEST_F(OnDemandConnectionManagerTest, onRequestProposal) {
  consensus::Round round;
  boost::optional<OnDemandConnectionManager::ProposalType> oproposal =
      OnDemandConnectionManager::ProposalType{};
  auto proposal = oproposal.value().get();
  EXPECT_CALL(*connections[OnDemandConnectionManager::kIssuer],
              onRequestProposal(round))
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
  consensus::Round round;
  boost::optional<OnDemandConnectionManager::ProposalType> oproposal;
  EXPECT_CALL(*connections[OnDemandConnectionManager::kIssuer],
              onRequestProposal(round))
      .WillOnce(Return(ByMove(std::move(oproposal))));

  auto result = manager->onRequestProposal(round);

  ASSERT_FALSE(result);
}

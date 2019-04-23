/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ordering/impl/ordering_gate_cache/on_demand_resend_strategy.hpp"

#include <gtest/gtest.h>
#include <boost/range/combine.hpp>
#include "module/irohad/ordering/mock_on_demand_os_notification.hpp"
#include "module/irohad/ordering/ordering_mocks.hpp"
#include "module/shared_model/interface_mocks.hpp"
#include "ordering/impl/on_demand_common.hpp"

using namespace iroha;
using namespace iroha::ordering;
using namespace iroha::ordering::transport;
using namespace iroha::consensus;

using ::testing::_;
using ::testing::ByMove;
using ::testing::Ref;
using ::testing::Return;

ACTION_P(CreateAndSave, var) {
  auto result = std::make_unique<MockOdOsNotification>();
  *var = result.get();
  return std::unique_ptr<OdOsNotification>(std::move(result));
}

struct OnDemandResendStrategyTest : public ::testing::Test {
  void SetUp() override {
    factory = std::make_shared<MockOdOsNotificationFactory>();
    strategy = std::make_shared<OnDemandResendStrategy>();

    auto set = [this](auto &field, auto &ptr, auto &conn) {
      field = std::make_shared<MockPeer>();

      EXPECT_CALL(*factory, create(Ref(*field)))
          .WillRepeatedly(CreateAndSave(&ptr));

      conn = factory->create(*field);
    };

    for (auto &&triple :
         boost::combine(cpeers.peers, connections, connections_obj.peers)) {
      set(boost::get<0>(triple), boost::get<1>(triple), boost::get<2>(triple));
    }
  }

  OnDemandConnectionManager::CurrentConnections connections_obj;
  OnDemandConnectionManager::CurrentPeers cpeers;
  OnDemandConnectionManager::PeerCollectionType<MockOdOsNotification *>
      connections;
  std::shared_ptr<MockOdOsNotificationFactory> factory;
  std::shared_ptr<OnDemandResendStrategy> strategy;
};

/**
 * @given OnDemandResendStrategy instance
 * @when same batch is fed to the instance twice
 * @then first feeding succeeds, second one fails
 */
TEST_F(OnDemandResendStrategyTest, Feed) {
  shared_model::interface::types::HashType hash("hash");
  auto batch = createMockBatchWithHash(hash);
  EXPECT_CALL(*batch, Equals(_)).Times(1).WillRepeatedly(Return(true));
  ASSERT_TRUE(strategy->feed(batch));
  ASSERT_FALSE(strategy->feed(batch));
}

/**
 * @given OnDemandResendStrategy instance
 * @when readyToUse is called before and after batch is fed
 * @then first call fails, second succeeds
 */
TEST_F(OnDemandResendStrategyTest, ReadyToUse) {
  shared_model::interface::types::HashType hash("hash");
  auto batch = createMockBatchWithHash(hash);
  ASSERT_FALSE(strategy->readyToUse(batch));
  strategy->feed(batch);
  EXPECT_CALL(*batch, Equals(_)).WillOnce(Return(true));
  ASSERT_TRUE(strategy->readyToUse(batch));
}

/**
 * @given OnDemandResendStrategy instance
 * @when sendBatches is called without any other prior calls
 * @then onBatches is called for all consumers
 */
TEST_F(OnDemandResendStrategyTest, ExtractNonExisting) {
  Round round(1, 1);
  strategy->setCurrentRound(round);
  shared_model::interface::types::HashType hash("hash");
  auto batch = createMockBatchWithHash(hash);

  EXPECT_CALL(*connections[OnDemandConnectionManager::kRejectRejectConsumer],
              onBatches(OnDemandConnectionManager::CollectionType{batch}))
      .Times(1);
  EXPECT_CALL(*connections[OnDemandConnectionManager::kRejectCommitConsumer],
              onBatches(OnDemandConnectionManager::CollectionType{batch}))
      .Times(1);
  EXPECT_CALL(*connections[OnDemandConnectionManager::kCommitRejectConsumer],
              onBatches(OnDemandConnectionManager::CollectionType{batch}))
      .Times(1);
  EXPECT_CALL(*connections[OnDemandConnectionManager::kCommitCommitConsumer],
              onBatches(OnDemandConnectionManager::CollectionType{batch}))
      .Times(1);

  strategy->sendBatches(OnDemandConnectionManager::CollectionType{batch},
                        connections_obj);
}

/**
 * @given OnDemandResendStrategy instance
 * @when feed and then sendBatches are called for the same batch
 * @then onBatches is called for all consumers
 */
TEST_F(OnDemandResendStrategyTest, ExtractNonReady) {
  Round round(1, 1);
  strategy->setCurrentRound(round);
  shared_model::interface::types::HashType hash("hash");
  auto batch = createMockBatchWithHash(hash);
  strategy->feed(batch);

  EXPECT_CALL(*batch, Equals(_)).WillOnce(Return(true));
  EXPECT_CALL(*connections[OnDemandConnectionManager::kRejectRejectConsumer],
              onBatches(OnDemandConnectionManager::CollectionType{batch}))
      .Times(1);
  EXPECT_CALL(*connections[OnDemandConnectionManager::kRejectCommitConsumer],
              onBatches(OnDemandConnectionManager::CollectionType{batch}))
      .Times(1);
  EXPECT_CALL(*connections[OnDemandConnectionManager::kCommitRejectConsumer],
              onBatches(OnDemandConnectionManager::CollectionType{batch}))
      .Times(1);
  EXPECT_CALL(*connections[OnDemandConnectionManager::kCommitCommitConsumer],
              onBatches(OnDemandConnectionManager::CollectionType{batch}))
      .Times(1);

  strategy->sendBatches(OnDemandConnectionManager::CollectionType{batch},
                        connections_obj);
}

/**
 * @given OnDemandResendStrategy instance
 * @when feed, readyToUse are called, current round is set for 2 commits to
 * future and then sendBatches is called for the same batch
 * @then onBatches is called for all consumers
 */
TEST_F(OnDemandResendStrategyTest, ExtractCommitCommit) {
  Round round(1, 1);
  strategy->setCurrentRound(round);
  shared_model::interface::types::HashType hash("hash");
  auto batch = createMockBatchWithHash(hash);
  strategy->feed(batch);
  EXPECT_CALL(*batch, Equals(_)).Times(2).WillRepeatedly(Return(true));
  strategy->readyToUse(batch);
  round = Round(3, 0);
  strategy->setCurrentRound(round);

  EXPECT_CALL(*connections[OnDemandConnectionManager::kRejectRejectConsumer],
              onBatches(OnDemandConnectionManager::CollectionType{batch}))
      .Times(1);
  EXPECT_CALL(*connections[OnDemandConnectionManager::kRejectCommitConsumer],
              onBatches(OnDemandConnectionManager::CollectionType{batch}))
      .Times(1);
  EXPECT_CALL(*connections[OnDemandConnectionManager::kCommitRejectConsumer],
              onBatches(OnDemandConnectionManager::CollectionType{batch}))
      .Times(1);
  EXPECT_CALL(*connections[OnDemandConnectionManager::kCommitCommitConsumer],
              onBatches(OnDemandConnectionManager::CollectionType{batch}))
      .Times(1);

  strategy->sendBatches(OnDemandConnectionManager::CollectionType{batch},
                        connections_obj);
}

/**
 * @given OnDemandResendStrategy instance
 * @when feed, readyToUse are called, current round is set for (commit, reject)
 * to future and then sendBatches is called for the same batch
 * @then onBatches is called for all consumers except (reject, commit)
 */
TEST_F(OnDemandResendStrategyTest, ExtractCommitReject) {
  Round round(1, 1);
  strategy->setCurrentRound(round);
  shared_model::interface::types::HashType hash("hash");
  auto batch = createMockBatchWithHash(hash);
  strategy->feed(batch);
  EXPECT_CALL(*batch, Equals(_)).Times(2).WillRepeatedly(Return(true));
  strategy->readyToUse(batch);
  round = Round(2, 1);
  strategy->setCurrentRound(round);

  EXPECT_CALL(*connections[OnDemandConnectionManager::kRejectRejectConsumer],
              onBatches(OnDemandConnectionManager::CollectionType{batch}))
      .Times(1);
  EXPECT_CALL(*connections[OnDemandConnectionManager::kCommitRejectConsumer],
              onBatches(OnDemandConnectionManager::CollectionType{batch}))
      .Times(1);
  EXPECT_CALL(*connections[OnDemandConnectionManager::kCommitCommitConsumer],
              onBatches(OnDemandConnectionManager::CollectionType{batch}))
      .Times(1);

  strategy->sendBatches(OnDemandConnectionManager::CollectionType{batch},
                        connections_obj);
}

/**
 * @given OnDemandResendStrategy instance
 * @when feed, readyToUse are called, current round is set for (reject, commit)
 * to future and then sendBatches is called for the same batch
 * @then onBatches is called for all consumers except (reject, commit)
 */
TEST_F(OnDemandResendStrategyTest, ExtractRejectCommit) {
  Round round(1, 1);
  strategy->setCurrentRound(round);
  shared_model::interface::types::HashType hash("hash");
  auto batch = createMockBatchWithHash(hash);
  strategy->feed(batch);
  EXPECT_CALL(*batch, Equals(_)).Times(2).WillRepeatedly(Return(true));
  strategy->readyToUse(batch);
  round = Round(2, 0);
  strategy->setCurrentRound(round);

  EXPECT_CALL(*connections[OnDemandConnectionManager::kRejectRejectConsumer],
              onBatches(OnDemandConnectionManager::CollectionType{batch}))
      .Times(1);
  EXPECT_CALL(*connections[OnDemandConnectionManager::kCommitRejectConsumer],
              onBatches(OnDemandConnectionManager::CollectionType{batch}))
      .Times(1);
  EXPECT_CALL(*connections[OnDemandConnectionManager::kCommitCommitConsumer],
              onBatches(OnDemandConnectionManager::CollectionType{batch}))
      .Times(1);

  strategy->sendBatches(OnDemandConnectionManager::CollectionType{batch},
                        connections_obj);
}

/**
 * @given OnDemandResendStrategy instance
 * @when feed, readyToUse are called, current round is set for (reject, reject)
 * to future and then sendBatches is called for the same batch
 * @then onBatches is called for (reject, commit) consumer
 */
TEST_F(OnDemandResendStrategyTest, ExtractRejectReject) {
  Round round(1, 1);
  strategy->setCurrentRound(round);
  shared_model::interface::types::HashType hash("hash");
  auto batch = createMockBatchWithHash(hash);
  strategy->feed(batch);
  EXPECT_CALL(*batch, Equals(_)).Times(2).WillRepeatedly(Return(true));
  strategy->readyToUse(batch);
  round = Round(1, 3);
  strategy->setCurrentRound(round);

  EXPECT_CALL(*connections[OnDemandConnectionManager::kRejectRejectConsumer],
              onBatches(OnDemandConnectionManager::CollectionType{batch}))
      .Times(1);

  strategy->sendBatches(OnDemandConnectionManager::CollectionType{batch},
                        connections_obj);
}

/**
 * @given OnDemandResendStrategy instance
 * @when feed, remove and feed are called for the same batch
 * @then both feed calls succeed
 */
TEST_F(OnDemandResendStrategyTest, Remove) {
  shared_model::interface::types::HashType hash("hash");
  auto tx = createMockTransactionWithHash(hash);
  auto batch = createMockBatchWithTransactions({tx}, "");
  ASSERT_TRUE(strategy->feed(batch));
  strategy->remove({hash});
  ASSERT_TRUE(strategy->feed(batch));
}

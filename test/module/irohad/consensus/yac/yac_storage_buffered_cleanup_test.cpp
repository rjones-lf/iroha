/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "consensus/yac/storage/yac_storage_cleanup_strategy_impl.hpp"

#include <gtest/gtest.h>
#include <memory>

using namespace iroha::consensus::yac;

class BufferedCleanupStrategyTest : public ::testing::Test {
 public:
  BufferedCleanupStrategy::QueueSizeType max_queue_size_;
  std::shared_ptr<CleanupStrategy> strategy_;

  void SetUp() override {
    const BufferedCleanupStrategy::QueueSizeType kNumberOfSavedRounds = 10;
    init(kNumberOfSavedRounds,
         BufferedCleanupStrategy::RoundType(1, 0),
         std::queue<BufferedCleanupStrategy::RoundType>());
  }

  void init(BufferedCleanupStrategy::QueueSizeType max_queue_size,
            BufferedCleanupStrategy::RoundType round,
            BufferedCleanupStrategy::QueueType queue) {
    max_queue_size_ = max_queue_size;
    strategy_ =
        std::make_shared<BufferedCleanupStrategy>(max_queue_size, round, queue);
  }

  RejectMessage makeMockReject() {
    return RejectMessage({});
  }
};

/**
 * @given initialized buffered strategy with defaults
 * @when  invoke finalize
 * @then  check that outcome:
 *        - is empty when queue is not full
 *        - has appropriate round for removing
 */
TEST_F(BufferedCleanupStrategyTest, FinalizeNotFullCheck) {
  for (iroha::consensus::RejectRoundType i = 0; i < max_queue_size_; ++i) {
    // no one element shouldn't be removed
    ASSERT_FALSE(strategy_->finalize({1, i}, makeMockReject()));
  }
  for (iroha::consensus::RejectRoundType i = max_queue_size_;
       i < 2 * max_queue_size_;
       ++i) {
    auto outcome = strategy_->finalize({1, i}, makeMockReject());
    // check that old elements are removed properly
    ASSERT_TRUE(outcome) << "fail on round " << i;
    ASSERT_EQ(1, outcome->size());
    ASSERT_EQ(iroha::consensus::Round(1, i - max_queue_size_), outcome->at(0));
  }
}

/**
 * @given initialized buffered strategy with some state
 * @when  invoke shouldCreateRound for
 *          - further
 *          - previous rounds
 * @then  check that further rounds may be created and previous not
 */
TEST_F(BufferedCleanupStrategyTest, ShouldCreateRoundCheck) {
  for (iroha::consensus::RejectRoundType i = 0; i < max_queue_size_; ++i) {
    strategy_->finalize({1, i}, makeMockReject());
  }

  for (iroha::consensus::RejectRoundType i = max_queue_size_;
       i < 2 * max_queue_size_ - 1;
       ++i) {
    ASSERT_TRUE(strategy_->shouldCreateRound({1, i}));
    ASSERT_FALSE(strategy_->shouldCreateRound(
        iroha::consensus::Round(1, i - max_queue_size_)));
  }
}

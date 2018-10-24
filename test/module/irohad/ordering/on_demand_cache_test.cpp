/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ordering/impl/og_cache/on_demand_cache.hpp"

#include <gtest/gtest.h>

#include "framework/batch_helper.hpp"

using namespace iroha::ordering::cache;
using ::testing::ElementsAre;
using ::testing::IsEmpty;
using ::testing::UnorderedElementsAre;

/**
 * @given empty og cache
 * @when add to back is invoked with batch1 and batch2
 * @then back of the cache consists has batch1 and batch2
 */
TEST(OnDemandCacheTest, TestAddToBack) {
  OnDemandCache cache;

  auto now = iroha::time::now();

  auto batch1 = std::shared_ptr<shared_model::interface::TransactionBatch>(
      framework::batch::createValidBatch(1, now));
  auto batch2 = std::shared_ptr<shared_model::interface::TransactionBatch>(
      framework::batch::createValidBatch(1, now + 1));

  cache.addToBack({batch1, batch2});

  ASSERT_THAT(cache.tail(), UnorderedElementsAre(batch1, batch2));
}

/**
 * @given og cache with single batch in each cell
 * @when up is invoked three times
 * @then all batches appear on the head of the queue
 */
TEST(OnDemandCacheTest, TestUp) {
  OnDemandCache cache;

  auto now = iroha::time::now();

  auto batch1 = std::shared_ptr<shared_model::interface::TransactionBatch>(
      framework::batch::createValidBatch(1, now));
  auto batch2 = std::shared_ptr<shared_model::interface::TransactionBatch>(
      framework::batch::createValidBatch(1, now + 1));
  auto batch3 = std::shared_ptr<shared_model::interface::TransactionBatch>(
      framework::batch::createValidBatch(1, now + 2));

  cache.addToBack({batch1});
  cache.up();
  /**
   * 1.
   * 2. {batch1}
   * 3.
   */
  ASSERT_THAT(cache.head(), IsEmpty());
  ASSERT_THAT(cache.tail(), IsEmpty());

  cache.addToBack({batch2});
  cache.up();
  /**
   * 1. {batch1}
   * 2. {batch2}
   * 3.
   */
  ASSERT_THAT(cache.head(), ElementsAre(batch1));
  ASSERT_THAT(cache.tail(), IsEmpty());

  cache.addToBack({batch3});
  cache.up();
  /**
   * 1. {batch1, batch2}
   * 2. {batch3}
   * 3.
   */
  ASSERT_THAT(cache.head(), UnorderedElementsAre(batch1, batch2));
  ASSERT_THAT(cache.tail(), IsEmpty());

  cache.up();
  /**
   * 1. {batch1, batch2, batch3}
   * 2.
   * 3.
   */
  ASSERT_THAT(cache.head(), UnorderedElementsAre(batch1, batch2, batch3));
  ASSERT_THAT(cache.tail(), IsEmpty());
}

/**
 * @given og cache with batch on the top
 * @when clearFrontAndGet is invoked on that cache
 * @then result contains that batch AND batch is removed from the front
 */
TEST(OnDemandCache, TestClearFrontAndGet) {
  OnDemandCache cache;

  auto batch1 = std::shared_ptr<shared_model::interface::TransactionBatch>(
      framework::batch::createValidBatch(1));

  cache.addToBack({batch1});
  ASSERT_THAT(cache.tail(), ElementsAre(batch1));
  /**
   * 1.
   * 2.
   * 3. {batch1}
   */

  // put {batch1} on the top
  cache.up();
  cache.up();
  /**
   * 1. {batch1}
   * 2.
   * 3.
   */

  // check that we have {batch1} on the top
  ASSERT_EQ(cache.head().size(), 1);
  ASSERT_THAT(cache.head(), ElementsAre(batch1));

  auto batchFromTop = cache.clearFrontAndGet();
  /**
   * 1.
   * 2.
   * 3.
   */
  ASSERT_THAT(batchFromTop, ElementsAre(batch1));
  ASSERT_THAT(cache.head(), IsEmpty());
}

/**
 * @given cache with batch1, batch2, and batch3 on the top
 * @when remove({batch2, batch3}) is invoked
 * @then only batch1 remains on the head of the queue
 */
TEST(OnDemandCache, Remove) {
  OnDemandCache cache;

  auto batch1 = std::shared_ptr<shared_model::interface::TransactionBatch>(
      framework::batch::createValidBatch(1));
  auto batch2 = std::shared_ptr<shared_model::interface::TransactionBatch>(
      framework::batch::createValidBatch(1));
  auto batch3 = std::shared_ptr<shared_model::interface::TransactionBatch>(
      framework::batch::createValidBatch(1));

  cache.addToBack({batch1, batch2, batch3});
  cache.up();
  cache.up();
  /**
   * 1. {batch1, batch2, batch3}
   * 2.
   * 3.
   */
  ASSERT_THAT(cache.head(), UnorderedElementsAre(batch1, batch2, batch3));

  cache.remove({batch2, batch3});
  /**
   * 1. {batch1}
   * 2.
   * 3.
   */
  ASSERT_THAT(cache.head(), ElementsAre(batch1));
}

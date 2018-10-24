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

TEST(OnDemandCacheTest, TestAddToBack) {
  OnDemandCache cache;

  auto now = iroha::time::now();

  auto batch1 = std::shared_ptr<shared_model::interface::TransactionBatch>(
      framework::batch::createValidBatch(1, now));

  cache.addToBack({batch1});

  ASSERT_THAT(cache.back(), ElementsAre(batch1));

  auto batch2 = std::shared_ptr<shared_model::interface::TransactionBatch>(
      framework::batch::createValidBatch(1, now + 1));
  cache.addToBack({batch2});

  ASSERT_THAT(cache.back(), UnorderedElementsAre(batch1, batch2));
}

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
  ASSERT_THAT(cache.front(), IsEmpty());
  ASSERT_THAT(cache.back(), IsEmpty());

  cache.addToBack({batch2});
  cache.up();
  /**
   * 1. {batch1}
   * 2. {batch2}
   * 3.
   */
  ASSERT_THAT(cache.front(), ElementsAre(batch1));
  ASSERT_THAT(cache.back(), IsEmpty());

  cache.addToBack({batch3});
  cache.up();
  /**
   * 1. {batch1, batch2}
   * 2.
   * 3.
   */
  ASSERT_THAT(cache.front(), UnorderedElementsAre(batch1, batch2));
  ASSERT_THAT(cache.back(), IsEmpty());

  cache.up();
  /**
   * 1. {batch1, batch2, batch3}
   * 2.
   * 3.
   */
  ASSERT_THAT(cache.front(), UnorderedElementsAre(batch1, batch2, batch3));
  ASSERT_THAT(cache.back(), IsEmpty());
}

TEST(OnDemandCache, TestClearFrontAndGet) {
  OnDemandCache cache;

  auto batch1 = std::shared_ptr<shared_model::interface::TransactionBatch>(
      framework::batch::createValidBatch(1));

  cache.addToBack({batch1});
  ASSERT_THAT(cache.back(), ElementsAre(batch1));
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
  ASSERT_EQ(cache.front().size(), 1);
  ASSERT_THAT(cache.front(), ElementsAre(batch1));

  auto batchFromTop = cache.clearFrontAndGet();
  /**
   * 1.
   * 2.
   * 3.
   */
  ASSERT_THAT(batchFromTop, ElementsAre(batch1));
  ASSERT_THAT(cache.front(), IsEmpty());
}

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
  ASSERT_THAT(cache.front(), UnorderedElementsAre(batch1, batch2, batch3));

  cache.remove({batch2, batch3});
  /**
   * 1. {batch1}
   * 2.
   * 3.
   */
  ASSERT_THAT(cache.front(), ElementsAre(batch1));
}

/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ordering/impl/ordering_gate_cache/on_demand_cache.hpp"

#include <gtest/gtest.h>

#include "framework/batch_helper.hpp"
#include "module/shared_model/interface_mocks.hpp"

using namespace iroha::ordering::cache;
using ::testing::ByMove;
using ::testing::ElementsAre;
using ::testing::IsEmpty;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::UnorderedElementsAre;
using namespace framework::batch;

/**
 * @given empty og cache
 * @when add to back is invoked with batch1 and batch2
 * @then back of the cache consists has batch1 and batch2
 */
TEST(OnDemandCacheTest, TestAddToBack) {
  OnDemandCache cache;

  shared_model::interface::types::HashType hash1("hash1");
  auto batch1 = createMockBatchWithHash(hash1);

  shared_model::interface::types::HashType hash2("hash2");
  auto batch2 = createMockBatchWithHash(hash2);

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

  shared_model::interface::types::HashType hash1("hash1");
  shared_model::interface::types::HashType hash2("hash2");
  shared_model::interface::types::HashType hash3("hash3");

  auto batch1 = createMockBatchWithHash(hash1);
  auto batch2 = createMockBatchWithHash(hash2);
  auto batch3 = createMockBatchWithHash(hash3);

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

  shared_model::interface::types::HashType hash("hash");
  auto batch1 = createMockBatchWithHash(hash);

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

  shared_model::interface::types::HashType hash1("hash1");
  shared_model::interface::types::HashType hash2("hash2");
  shared_model::interface::types::HashType hash3("hash3");

  auto batch1 = createMockBatchWithHash(hash1);
  auto batch2 = createMockBatchWithHash(hash2);
  auto batch3 = createMockBatchWithHash(hash3);

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

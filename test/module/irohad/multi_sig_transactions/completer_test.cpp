/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>
#include <chrono>
#include "module/irohad/multi_sig_transactions/mst_test_helpers.hpp"
#include "multi_sig_transactions/state/mst_state.hpp"

using namespace iroha;

/**
 * @given batch with 3 transactions: first one with quorum 1 and 1 signature,
 * second one with quorum 2 and 2 signatures, third one with quorum 3 and 3
 * signatures
 * @when completer was called for the batch
 * @then batch is complete
 */
TEST(CompleterTest, BatchQuorumTestEnoughSignatures) {
  auto completer = std::make_shared<DefaultCompleter>(std::chrono::minutes(0));
  auto time = iroha::time::now();
  auto batch = makeTestBatch(
      txBuilder(1, time, 1), txBuilder(2, time, 2), txBuilder(3, time, 3));
  addSignatures(batch, 0, makeSignature("1", "1"));
  addSignatures(batch, 1, makeSignature("2", "1"));
  addSignatures(batch, 1, makeSignature("2", "2"));
  addSignatures(batch, 2, makeSignature("3", "1"));
  addSignatures(batch, 2, makeSignature("3", "2"));
  addSignatures(batch, 2, makeSignature("3", "3"));
  ASSERT_TRUE((*completer)(batch));
}

/**
 * @given batch with 3 transactions: first one with quorum 1 and 1 signature,
 * second one with quorum 2 and 1 signature, third one with quorum 3 and 3
 * signatures
 * @when completer was called for the batch
 * @then batch is not complete
 */
TEST(CompleterTest, BatchQuorumTestNotEnoughSignatures) {
  auto completer = std::make_shared<DefaultCompleter>(std::chrono::minutes(0));
  auto time = iroha::time::now();
  auto batch = makeTestBatch(
      txBuilder(1, time, 1), txBuilder(2, time, 2), txBuilder(3, time, 3));
  addSignatures(batch, 0, makeSignature("1", "1"));
  addSignatures(batch, 1, makeSignature("2", "1"));
  addSignatures(batch, 2, makeSignature("3", "1"));
  addSignatures(batch, 2, makeSignature("3", "2"));
  addSignatures(batch, 2, makeSignature("3", "3"));
  ASSERT_FALSE((*completer)(batch));
}

/**
 * @given batch with 3 transactions with now() creation time and completer with
 * 1 minute expiration time
 * @when completer with 2 minute gap was called for the batch
 * @then batch is expired
 */
TEST(CompleterTest, BatchExpirationTestExpired) {
  auto completer = std::make_shared<DefaultCompleter>(std::chrono::minutes(1));
  auto time = iroha::time::now();
  auto batch = makeTestBatch(
      txBuilder(1, time, 1), txBuilder(2, time, 2), txBuilder(3, time, 3));
  ASSERT_TRUE((*completer)(
      batch, time + std::chrono::minutes(2) / std::chrono::milliseconds(1)));
}

/**
 * @given batch with 3 transactions: first one in 2 minutes from now,
 * second one in 3 minutes from now, third one in 4 minutes from now and
 * completer with 5 minute expiration time
 * @when completer without time gap was called for the batch
 * @then batch is not expired
 */
TEST(CompleterTest, BatchExpirationTestNoExpired) {
  auto completer = std::make_shared<DefaultCompleter>(std::chrono::minutes(5));
  auto time = iroha::time::now();
  auto batch = makeTestBatch(
      txBuilder(
          1, time + std::chrono::minutes(2) / std::chrono::milliseconds(1), 1),
      txBuilder(
          2, time + std::chrono::minutes(3) / std::chrono::milliseconds(1), 2),
      txBuilder(
          3, time + std::chrono::minutes(4) / std::chrono::milliseconds(1), 3));
  ASSERT_FALSE((*completer)(batch, time));
}

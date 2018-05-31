/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>

#include "builders/protobuf/block.hpp"
#include "module/shared_model/builders/protobuf/test_transaction_builder.hpp"

using namespace shared_model::proto;

/**
 * @given BlockBuilder
 * @when Block with transactions is built using given BlockBuilder
 * @then no exception is thrown
 */
TEST(BlockBuilderTest, BlockWithTransactions) {
  shared_model::proto::Transaction tx =
      TestTransactionBuilder()
          .createdTime(iroha::time::now())
          .creatorAccountId("admin@test")
          .addAssetQuantity("admin@test", "coin#test", "1.0")
          .build();

  ASSERT_NO_THROW(
      BlockBuilder()
          .createdTime(iroha::time::now())
          .prevHash(shared_model::crypto::Hash(std::string(32, '0')))
          .height(1)
          .transactions(std::vector<decltype(tx)>{tx})
          .build());
}

/**
 * @given BlockBuilder
 * @when Block with no transactions is built using given BlockBuilder
 * @then exception is thrown
 */
// TODO IR-1295 10.05.18 Remove disabled when 1295 is
// implemented
TEST(BlockBuilderTest, DISABLED_BlockWithNoTransactions) {
  ASSERT_ANY_THROW(
      BlockBuilder()
          .createdTime(iroha::time::now())
          .prevHash(shared_model::crypto::Hash(std::string(32, '0')))
          .height(1)
          .transactions(std::vector<shared_model::proto::Transaction>())
          .build());
}

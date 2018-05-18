/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>
#include "builders/protobuf/proposal.hpp"
#include "module/shared_model/builders/protobuf/test_transaction_builder.hpp"

using namespace shared_model::proto;

/**
 * @given ProposalBuilder
 * @when Proposal with transactions is built using given ProposalBuilder
 * @then no exception is thrown
 */
TEST(ProposalBuilderTest, ProposalWithTransactions) {
  shared_model::proto::Transaction tx =
      TestTransactionBuilder()
          .createdTime(iroha::time::now())
          .creatorAccountId("admin@test")
          .addAssetQuantity("admin@test", "coin#test", "1.0")
          .build();

  ASSERT_NO_THROW(
      ProposalBuilder()
          .createdTime(iroha::time::now())
          .height(1)
          .transactions(std::vector<decltype(tx)>{tx})
          .build());
}

/**
 * @given ProposalBuilder
 * @when Proposal with no transactions is built using given ProposalBuilder
 * @then exception is thrown
 */
TEST(ProposalBuilderTest, ProposalWithNoTransactions) {
  ASSERT_ANY_THROW(
      ProposalBuilder()
          .createdTime(iroha::time::now())
          .height(1)
          .transactions(std::vector<shared_model::proto::Transaction>())
          .build());
}

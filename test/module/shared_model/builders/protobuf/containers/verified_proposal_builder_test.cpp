/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>
#include "builders/protobuf/verified_proposal.hpp"
#include "module/shared_model/builders/protobuf/test_transaction_builder.hpp"

using namespace shared_model::proto;

/**
 * @given VerifiedProposalBuilder
 * @when VerifiedProposal with transactions is built using given
 * VerifiedProposalBuilder
 * @then no exception is thrown
 */
TEST(VerifiedProposalBuilderTest, VerifiedProposalWithTransactions) {
  shared_model::proto::Transaction tx =
      TestTransactionBuilder()
          .createdTime(iroha::time::now())
          .creatorAccountId("admin@test")
          .addAssetQuantity("admin@test", "coin#test", "1.0")
          .build();

  ASSERT_NO_THROW(VerifiedProposalBuilder()
                      .createdTime(iroha::time::now())
                      .height(1)
                      .transactions(std::vector<decltype(tx)>{tx})
                      .build());
}

/**
 * @given VerifiedProposalBuilder
 * @when VerifiedProposal with no transactions is built using given
 * VerifiedProposalBuilder
 * @then no exception is thrown
 */
TEST(VerifiedProposalBuilderTest, VerifiedProposalWithNoTransactions) {
  ASSERT_NO_THROW(
      VerifiedProposalBuilder()
          .createdTime(iroha::time::now())
          .height(1)
          .transactions(std::vector<shared_model::proto::Transaction>())
          .build());
}

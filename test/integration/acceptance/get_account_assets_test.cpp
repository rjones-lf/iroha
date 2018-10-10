/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>
#include "backend/protobuf/transaction.hpp"
#include "builders/protobuf/queries.hpp"
#include "framework/integration_framework/integration_test_framework.hpp"
#include "framework/specified_visitor.hpp"
#include "integration/acceptance/acceptance_fixture.hpp"
#include "utils/query_error_response_visitor.hpp"

using namespace integration_framework;
using namespace shared_model;

class GetAccountAssets : public AcceptanceFixture {
 public:
  /**
   * Creates the transaction with the user creation commands
   * @param perms are the permissions of the user
   * @return built tx and a hash of its payload
   */
  auto makeUserWithPerms(const interface::RolePermissionSet &perms = {
                             interface::permissions::Role::kGetMyAccAst}) {
    auto new_perms = perms;
    new_perms.set(interface::permissions::Role::kAddAssetQty);
    new_perms.set(interface::permissions::Role::kSubtractAssetQty);
    return AcceptanceFixture::makeUserWithPerms(kNewRole, new_perms);
  }

  /// Create command for adding assets
  auto addAssets() {
    return complete(
        AcceptanceFixture::baseTx().addAssetQuantity(kAssetId, "1"));
  }

  /// Create command for removing assets
  auto removeAssets() {
    return complete(
        AcceptanceFixture::baseTx().subtractAssetQuantity(kAssetId, "1"));
  }

  /**
   * Creates valid GetAccountAssets query of current user
   * @param hash of the tx for querying
   * @return built query
   */
  auto makeQuery() {
    return complete(baseQry().queryCounter(1).getAccountAssets(kUserId));
  }

  static auto checkAccountAssets(int quantity) {
    return
        [quantity](const shared_model::proto::QueryResponse &query_response) {
          ASSERT_NO_THROW({
            const auto &resp = boost::apply_visitor(
                framework::SpecifiedVisitor<interface::AccountAssetResponse>(),
                query_response.get());

            ASSERT_EQ(resp.accountAssets().size(), quantity);
          }) << query_response.toString();
        };
  }

  const std::string kNewRole = "rl";
};

/**
 * @given a user with all required permissions
 * @when GetAccountAssets is queried on the user
 * @then there is an AccountAssetResponse
 */
TEST_F(GetAccountAssets, AddedAssets) {
  auto check_single_asset = checkAccountAssets(1);

  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .skipProposal()
      .skipBlock()
      .sendTxAwait(
          addAssets(),
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); })
      .sendQuery(makeQuery(), check_single_asset);
}

/**
 * @given a user with all required permissions
 * @when GetAccountAssets is queried on the user
 * @then there is an AccountAssetResponse
 */
TEST_F(GetAccountAssets, RemovedAssets) {
  auto check_single_asset = checkAccountAssets(1);

  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .skipProposal()
      .skipBlock()
      .sendTx(addAssets())
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); })
      .sendTxAwait(
          removeAssets(),
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); })
      .sendQuery(makeQuery(), check_single_asset);
}

/**
 * @given a user with all required permissions
 * @when GetAccountAssets is queried on the user
 * @then there is an AccountAssetResponse
 */
TEST_F(GetAccountAssets, NonAddedAssets) {
  auto check_zero_assets = checkAccountAssets(0);

  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(makeUserWithPerms())
      .skipProposal()
      .skipVerifiedProposal()
      .skipBlock()
      .sendQuery(makeQuery(), check_zero_assets);
}

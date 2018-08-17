/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "framework/integration_framework/integration_test_framework.hpp"
#include "integration/acceptance/acceptance_fixture.hpp"

using namespace shared_model;
using namespace integration_framework;
using namespace shared_model::interface::permissions;

class AccountAssetTxsFixture : public AcceptanceFixture {
 public:
  std::unique_ptr<IntegrationTestFramework> itf_;

  const interface::types::AccountNameType target_account_name_ = "target";

  interface::types::AccountIdType

  IntegrationTestFramework &prepareState(
      const interface::RolePermissionSet &permissions) {
    auto perms = permissions;
    perms.set(interface::permissions::Role::kAddAssetQty);
    perms.set(interface::permissions::Role::kSubtractAssetQty);
    perms.set(interface::permissions::Role::kReceive);
    perms.set(interface::permissions::Role::kTransfer);
    return itf_->sendTx(makeUserWithPerms(perms))
        .skipProposal()
        .checkBlock(
            [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); });
  }



 protected:
  void SetUp() override {
    itf_ = std::make_unique<IntegrationTestFramework>(1);
    itf_->setInitialState(kAdminKeypair);
  }

  //  void TearDown() override {}
};

TEST_F(AccountAssetTxsFixture, Basic) {
  prepareState({interface::permissions::Role::kSetQuorum})
      .sendTx(complete(baseTx().setAccountQuorum(kUserId, 1)))
      .skipProposal()
      .checkBlock(
          [](auto &block) { ASSERT_EQ(block->transactions().size(), 1); });
  auto x = 2;
  ASSERT_EQ(x, 2);
}

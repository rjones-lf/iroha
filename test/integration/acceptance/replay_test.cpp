/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>
#include "framework/integration_framework/integration_test_framework.hpp"
#include "integration/acceptance/acceptance_fixture.hpp"

using namespace integration_framework;
using namespace shared_model;
using namespace common_constants;

#define check(i) [](auto &block) { ASSERT_EQ(block->transactions().size(), i); }

class ReplayFixture : public AcceptanceFixture {
 public:
  ReplayFixture() : itf(1) {}

  void SetUp() override {
    auto create_user_tx =
        complete(baseTx(kAdminId)
                     .createAccount(kUser, kDomain, kUserKeypair.publicKey())
                     .addAssetQuantity(kAssetId, "10000.0"),
                 kAdminKeypair);
    itf.setInitialState(kAdminKeypair).sendTxAwait(create_user_tx, check(1));
  }

  IntegrationTestFramework itf;
};

// TODO igor-egorov, 07 Nov 2018, enable the test, IR-1773 & IR-1838
/**
 * Basic case of transaction replay attack
 * @given an initialized ITF and a transaction
 * @when the transaction is sent to ITF twice
 * @then the second submission should be rejected
 */
TEST_F(ReplayFixture, DISABLED_BasicTxReplay) {
  auto transfer_tx = complete(
      baseTx(kAdminId).transferAsset(kAdminId, kUserId, kAssetId, "", "1.0"),
      kAdminKeypair);

  itf.sendTxAwait(transfer_tx, check(1));  // should be committed
  itf.sendTxAwait(transfer_tx, check(0));  // should not
}

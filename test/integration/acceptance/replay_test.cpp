/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>
#include "framework/integration_framework/integration_test_framework.hpp"
#include "integration/acceptance/acceptance_fixture.hpp"
#include "interfaces/permissions.hpp"

using namespace integration_framework;
using namespace shared_model;
using namespace common_constants;
using namespace shared_model::interface::permissions;

class ReplayFixture : public AcceptanceFixture {
 public:
  ReplayFixture() : itf(1), kReceiverRole("receiver") {}

  void SetUp() override {
    auto create_user_tx =
        complete(baseTx(kAdminId)
                     .createAccount(kUser, kDomain, kUserKeypair.publicKey())
                     .createRole(kReceiverRole, {Role::kReceive})
                     .appendRole(kUserId, kReceiverRole)
                     .addAssetQuantity(kAssetId, "10000.0"),
                 kAdminKeypair);
    itf.setInitialState(kAdminKeypair)
        .sendTxAwait(create_user_tx, CHECK_TXS_QUANTITY(1));
  }

  IntegrationTestFramework itf;
  const interface::types::RoleIdType kReceiverRole;
};

/**
 * Basic case of transaction replay attack.
 * OG/OS should not pass replayed transaction
 * @given an initialized ITF and a transaction
 * @when the transaction is sent to ITF twice
 * @then the second submission should be rejected
 */
TEST_F(ReplayFixture, DISABLE_OrderingGateReplay) {
  /*
   * Disabled because now it is not clear how to test replays.
   * In case of replay Iroha will just produce a warning in std output.
   * Iroha would not throw anymore missed proposal exception.
   */

  auto transfer_tx = complete(
      baseTx(kAdminId).transferAsset(kAdminId, kUserId, kAssetId, "", "1.0"),
      kAdminKeypair);

  itf.sendTxAwait(transfer_tx, CHECK_TXS_QUANTITY(1));  // should be committed
  itf.sendTx(transfer_tx);                              // should not
  EXPECT_THROW(itf.skipProposal(),
               std::runtime_error);  // missed proposal should be thrown here
  // TODO 2019-01-09 igor-egorov IR-152
  // redo without exception handling. Need to make ITF able to handle
  // "none" answer from ordering service when there is no proposal
}

/**
 * @given ITF with hacked OS that provides the same proposal twice
 * @when YAC accepts the proposal twice
 * @then a transaction from proposal would not be committed twice
 */
TEST_F(ReplayFixture, DISABLED_ConsensusReplay) {
  // TODO 2019-01-09 igor-egorov IR-153
}

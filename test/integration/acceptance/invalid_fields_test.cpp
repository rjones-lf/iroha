/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>
#include "block.pb.h"
#include "framework/integration_framework/integration_test_framework.hpp"
#include "integration/acceptance/acceptance_fixture.hpp"

using namespace integration_framework;
using namespace shared_model;

class InvalidField : public AcceptanceFixture {};

/**
 * @given tx with CreateAccount command and invalid signature size
 * @when send it
 * @then Torii returns stateless fail
 */
TEST_F(InvalidField, Signature) {
  auto tx = complete(baseTx()).getTransport();
  // extend signature to invalid size
  auto sig = tx.mutable_signatures(0)->mutable_signature();
  sig->resize(sig->size() + 1, 'a');

  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(proto::Transaction(tx), checkStatelessInvalid)
      .done();
}

/**
 * @given tx with CreateAccount command and invalid pub key size
 * @when send it
 * @then Torii returns stateless fail
 */
TEST_F(InvalidField, Pubkey) {
  auto tx = complete(baseTx()).getTransport();
  // extend public key to invalid size
  auto pkey = tx.mutable_signatures(0)->mutable_pubkey();
  pkey->resize(pkey->size() + 1, 'a');

  IntegrationTestFramework(1)
      .setInitialState(kAdminKeypair)
      .sendTx(proto::Transaction(tx), checkStatelessInvalid)
      .done();
}

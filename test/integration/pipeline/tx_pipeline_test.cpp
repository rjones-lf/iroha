/**
 * Copyright Soramitsu Co., Ltd. 2018 All Rights Reserved.
 * http://soramitsu.co.jp
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "framework/integration_framework/integration_test_framework.hpp"

using integration_framework::IntegrationTestFramework;

/**
 * @given some user
 * @when sending sample AddAssetQuantity transaction to the ledger
 * @then receive STATELESS_VALIDATION_SUCCESS status on that tx
 */
TEST(TransactionPipeline, SendTx) {
  iroha::model::generators::CommandGenerator gen;

  iroha::model::Transaction tx;
  tx.commands.push_back(gen.generateAddAssetQuantity(
      "user", "test", iroha::Amount().createFromString("0").value()));

  IntegrationTestFramework().setInitialState().sendTx(tx).done();
}

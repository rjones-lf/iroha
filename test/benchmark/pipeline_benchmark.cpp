/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <benchmark/benchmark.h>
#include <string>

#include "backend/protobuf/transaction.hpp"
#include "builders/protobuf/unsigned_proto.hpp"
#include "datetime/time.hpp"
#include "framework/integration_framework/integration_test_framework.hpp"
#include "framework/specified_visitor.hpp"
#include "module/shared_model/builders/protobuf/test_query_builder.hpp"
#include "module/shared_model/builders/protobuf/test_transaction_builder.hpp"
#include "utils/query_error_response_visitor.hpp"

const std::string kUser = "user";
const std::string kUserId = kUser + "@test";
const std::string kAmount = "1.0";
const std::string kAsset = "coin#test";
const shared_model::crypto::Keypair kAdminKeypair =
    shared_model::crypto::DefaultCryptoAlgorithmType::generateKeypair();
const shared_model::crypto::Keypair kUserKeypair =
    shared_model::crypto::DefaultCryptoAlgorithmType::generateKeypair();

auto baseTx() {
  return TestUnsignedTransactionBuilder().creatorAccountId(kUserId).createdTime(
      iroha::time::now());
}

TestUnsignedTransactionBuilder createUserWithPerms(
    const std::string &user,
    const shared_model::crypto::PublicKey &key,
    const std::string &role_id,
    const shared_model::interface::RolePermissionSet &perms) {
  const auto user_id = user + "@"
      + integration_framework::IntegrationTestFramework::kDefaultDomain;
  return TestUnsignedTransactionBuilder()
      .createAccount(
          user,
          integration_framework::IntegrationTestFramework::kDefaultDomain,
          key)
      .creatorAccountId(
          integration_framework::IntegrationTestFramework::kAdminId)
      .createdTime(iroha::time::now())
      .quorum(1)
      .detachRole(user_id,
                  integration_framework::IntegrationTestFramework::kDefaultRole)
      .createRole(role_id, perms)
      .appendRole(user_id, role_id);
}

const auto transactions = 100;
const auto commands = 100;

static void BM_AddAssetQuantity(benchmark::State &state) {
  integration_framework::IntegrationTestFramework itf(transactions);
  itf.setInitialState(kAdminKeypair);
  for (int i = 0; i < transactions; i++) {
    itf.sendTx(createUserWithPerms(
        kUser,
        kUserKeypair.publicKey(),
        "role",
        {shared_model::interface::permissions::Role::kAddAssetQty})
                   .build()
                   .signAndAddSignature(kAdminKeypair)
                   .finish());
  }
  itf.skipBlock().skipProposal();

  //  auto transaction = ;
  // define main benchmark loop
  while (state.KeepRunning()) {
    // define the code to be tested

    auto make_base = [&]() {
      auto base = baseTx();
      for (int i = 0; i < commands; i++) {
        base = base.addAssetQuantity(kAsset, kAmount);
      }
      return base.quorum(1).build()
          .signAndAddSignature(kUserKeypair).finish();
    };

    for (int i = 0; i < transactions; i++) {
      itf.sendTx(make_base());
    }
    itf.skipProposal().skipBlock();
  }
  itf.done();
}
// define benchmark
BENCHMARK(BM_AddAssetQuantity)->Unit(benchmark::kMillisecond);

/// That's all. More in documentation.

// don't forget to include this:
BENCHMARK_MAIN();

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
#include "module/shared_model/builders/protobuf/test_query_builder.hpp"
#include "module/shared_model/builders/protobuf/test_transaction_builder.hpp"

const std::string kUser = "user";
const std::string kUserId = kUser + "@test";
const shared_model::crypto::Keypair kAdminKeypair =
    shared_model::crypto::DefaultCryptoAlgorithmType::generateKeypair();
const shared_model::crypto::Keypair kUserKeypair =
    shared_model::crypto::DefaultCryptoAlgorithmType::generateKeypair();

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

static void BM_QueryAccount(benchmark::State &state) {
  integration_framework::IntegrationTestFramework itf(1);
  itf.setInitialState(kAdminKeypair);
  itf.sendTx(createUserWithPerms(
                 kUser,
                 kUserKeypair.publicKey(),
                 "role",
                 {shared_model::interface::permissions::Role::kGetAllAccounts})
                 .build()
                 .signAndAddSignature(kAdminKeypair)
                 .finish());

  itf.skipBlock().skipProposal();

  auto make_query = []() {
    return TestQueryBuilder()
        .createdTime(iroha::time::now())
        .creatorAccountId(kUserId)
        .queryCounter(1)
        .getAccount(kUserId)
        .build();
  };
  auto check = [](auto &status) {};

  // define main benchmark loop
  while (state.KeepRunning()) {
    // define the code to be tested
    itf.sendQuery(make_query(), check);
  }
  itf.done();
}
// define benchmark
BENCHMARK(BM_QueryAccount);

/// That's all. More in documentation.

// don't forget to include this:
BENCHMARK_MAIN();

/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <benchmark/benchmark.h>
#include <string>

#include "backend/protobuf/transaction.hpp"
#include "builders/protobuf/unsigned_proto.hpp"
#include "cryptography/crypto_provider/crypto_defaults.hpp"
#include "datetime/time.hpp"
#include "framework/integration_framework/integration_test_framework.hpp"
#include "integration/acceptance/acceptance_fixture.hpp"
#include "module/shared_model/builders/protobuf/test_transaction_builder.hpp"
#include "validators/permissions.hpp"

using namespace integration_framework;
using namespace shared_model;

const std::string kUser = "user";
const std::string kUserId = kUser + "@test";
const std::string kAmount = "1.0";
const crypto::Keypair kAdminKeypair =
    crypto::DefaultCryptoAlgorithmType::generateKeypair();
const crypto::Keypair kUserKeypair =
    crypto::DefaultCryptoAlgorithmType::generateKeypair();

class QueryBenchmark : public benchmark::Fixture {
 public:
  TestUnsignedTransactionBuilder createUser(
      const std::string &user, const shared_model::crypto::PublicKey &key) {
    return TestUnsignedTransactionBuilder()
        .createAccount(
            user,
            integration_framework::IntegrationTestFramework::kDefaultDomain,
            key)
        .creatorAccountId(
            integration_framework::IntegrationTestFramework::kAdminId)
        .createdTime(iroha::time::now())
        .quorum(1);
  }

  TestUnsignedTransactionBuilder createUserWithPerms(
      const std::string &user,
      const shared_model::crypto::PublicKey &key,
      const std::string &role_id,
      const shared_model::interface::RolePermissionSet &perms) {
    const auto user_id = user + "@"
        + integration_framework::IntegrationTestFramework::kDefaultDomain;
    return createUser(user, key)
        .detachRole(user_id,
                    integration_framework::IntegrationTestFramework::kDefaultRole)
        .createRole(role_id, perms)
        .appendRole(user_id, role_id);
  }
};

BENCHMARK_DEFINE_F(QueryBenchmark, GetAccount)(benchmark::State &state) {
  const std::string kAsset = "coin#test";

  auto make_perms = [this] {
    auto perms = shared_model::interface::RolePermissionSet(
        {shared_model::interface::permissions::Role::kAddAssetQty});
    return createUserWithPerms(kUser, kUserKeypair.publicKey(), "role", perms)
        .build()
        .signAndAddSignature(kAdminKeypair)
        .finish();
  };

  IntegrationTestFramework itf(1);
  itf.setInitialState(kAdminKeypair);

  for (int i = 0; i < 1; i++) {
    itf.sendTx(make_perms());
  }
  itf.skipProposal().skipBlock();

  //  auto transaction = ;
  // define main benchmark loop
  while (state.KeepRunning()) {
    // define the code to be tested

    std::string empty_string;
  }
  itf.done();
}
// define benchmark
// BENCHMARK(BM_QueryAccount)->Unit(benchmark::kMillisecond);
BENCHMARK_REGISTER_F(QueryBenchmark, GetAccount)->UseManualTime();

/// That's all. More in documentation.

// don't forget to include this:
BENCHMARK_MAIN();

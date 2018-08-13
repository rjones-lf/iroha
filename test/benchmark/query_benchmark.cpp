/**
 * Copyright Soramitsu Co., Ltd. 2017 All Rights Reserved.
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

///
/// Documentation is at https://github.com/google/benchmark
///
/// General recommendations:
///  - build with -DCMAKE_BUILD_TYPE=Release
///  - disable CPU scaling (frequency changes depending on workload)
///  - put initialization code in fixtures
///  - write meaningful names for benchmarks
///

#include <benchmark/benchmark.h>
#include <string>

#include "backend/protobuf/transaction.hpp"
#include "builders/protobuf/unsigned_proto.hpp"
#include "cryptography/crypto_provider/crypto_defaults.hpp"
#include "datetime/time.hpp"
#include "framework/integration_framework/integration_test_framework.hpp"
#include "module/shared_model/builders/protobuf/test_query_builder.hpp"
#include "module/shared_model/builders/protobuf/test_transaction_builder.hpp"
#include "validators/permissions.hpp"

const std::string kUser = "user";
const std::string kUserId = kUser + "@test";
const std::string kAmount = "1.0";
const shared_model::crypto::Keypair kAdminKeypair =
    shared_model::crypto::DefaultCryptoAlgorithmType::generateKeypair();
const shared_model::crypto::Keypair kUserKeypair =
    shared_model::crypto::DefaultCryptoAlgorithmType::generateKeypair();

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

/// Test how long is empty std::string creation
// define a static function, which accepts 'state'
// function's name = benchmark's name
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

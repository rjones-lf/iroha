/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <benchmark/benchmark.h>
#include <string>

#include "backend/protobuf/transaction.hpp"
#include "benchmark/bm_utils.hpp"
#include "framework/specified_visitor.hpp"
#include "module/shared_model/builders/protobuf/test_query_builder.hpp"
#include "utils/query_error_response_visitor.hpp"

using namespace benchmark::utils;

const std::string kUser = "user";
const std::string kUserId = kUser + "@test";
const shared_model::crypto::Keypair kAdminKeypair =
    shared_model::crypto::DefaultCryptoAlgorithmType::generateKeypair();
const shared_model::crypto::Keypair kUserKeypair =
    shared_model::crypto::DefaultCryptoAlgorithmType::generateKeypair();

/**
 * This benchmark executes get account query in order to measure query execution
 * performance
 */
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
    return TestUnsignedQueryBuilder()
        .createdTime(iroha::time::now())
        .creatorAccountId(kUserId)
        .queryCounter(1)
        .getAccount(kUserId)
        .build()
        .signAndAddSignature(kUserKeypair)
        .finish();
  };

  auto check = [](auto &status) {
    boost::apply_visitor(
        framework::SpecifiedVisitor<
            const shared_model::interface::AccountResponse &>(),
        status.get());
  };

  itf.sendQuery(make_query(), check);

  while (state.KeepRunning()) {
    itf.sendQuery(make_query());
  }
  itf.done();
}
BENCHMARK(BM_QueryAccount)->Unit(benchmark::kMicrosecond);

BENCHMARK_MAIN();

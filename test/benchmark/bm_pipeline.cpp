/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <benchmark/benchmark.h>
#include <string>

#include "backend/protobuf/transaction.hpp"
#include "benchmark/bm_utils.hpp"
#include "builders/protobuf/unsigned_proto.hpp"
#include "datetime/time.hpp"
#include "framework/integration_framework/integration_test_framework.hpp"
#include "framework/specified_visitor.hpp"
#include "module/shared_model/builders/protobuf/test_query_builder.hpp"
#include "module/shared_model/builders/protobuf/test_transaction_builder.hpp"
#include "utils/query_error_response_visitor.hpp"

using namespace benchmark::utils;

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

const auto proposal_size = 100;
const auto transaction_size = 100;

/**
 * This benchmark runs execution of the add asset quantity command in order to
 * measure execution performance
 * @param state
 */
static void BM_AddAssetQuantity(benchmark::State &state) {
  integration_framework::IntegrationTestFramework itf(
      proposal_size,
      boost::none,
      [](auto &) {},
      false,
      (boost::filesystem::temp_directory_path()
       / boost::filesystem::unique_path())
          .string(),
      std::chrono::hours(1),
      std::chrono::hours(1));
  itf.setInitialState(kAdminKeypair);
  for (int i = 0; i < proposal_size; i++) {
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

  while (state.KeepRunning()) {
    auto make_base = [&]() {
      auto base = baseTx();
      for (int i = 0; i < transaction_size; i++) {
        base = base.addAssetQuantity(kAsset, kAmount);
      }
      return base.quorum(1).build().signAndAddSignature(kUserKeypair).finish();
    };

    for (int i = 0; i < proposal_size; i++) {
      itf.sendTx(make_base());
    }
    itf.skipProposal().skipBlock();
  }
  itf.done();
}

BENCHMARK(BM_AddAssetQuantity)->Unit(benchmark::kMillisecond);

BENCHMARK_MAIN();

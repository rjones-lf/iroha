/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * When passing through the pipeline, we need to form proposal
 * out of transactions and then make blocks out of it. This results in several
 * transformations of underlying transport implementation,
 * which can be visibly slow.
 *
 * The purpose of this benchmark is to keep track of performance costs related
 * to blocks and proposals copying/moving.
 * 
 * Each benchmark runs transaction() and commands() call to 
 * initialize possibly lazy fields.
 */

#include <benchmark/benchmark.h>

#include "backend/protobuf/block.hpp"
#include "datetime/time.hpp"
#include "module/shared_model/builders/protobuf/test_block_builder.hpp"
#include "module/shared_model/builders/protobuf/test_proposal_builder.hpp"
#include "module/shared_model/builders/protobuf/test_transaction_builder.hpp"

/// number of commands in a single transaction
constexpr int number_of_commands = 5;

/// number of transactions in a single block
constexpr int number_of_txs = 100;

class BlockBenchmark : public benchmark::Fixture {
 public:
  // Block cannot be copy-assigned, that's why the state is kept in a builder
  TestBlockBuilder complete_builder;

  /**
   * Initialize block builder for benchmarks
   */
  void SetUp(benchmark::State &st) override {
    TestBlockBuilder builder;
    TestTransactionBuilder txbuilder;

    auto base_tx = txbuilder.createdTime(iroha::time::now()).quorum(1);

    for (int i = 0; i < number_of_commands; i++) {
      base_tx.transferAsset("player@one", "player@two", "coin", "", "5.00");
    }

    std::vector<shared_model::proto::Transaction> txs;

    for (int i = 0; i < number_of_txs; i++) {
      txs.push_back(base_tx.build());
    }

    complete_builder =
        builder.createdTime(iroha::time::now()).height(1).transactions(txs);
  }
};

class ProposalBenchmark : public benchmark::Fixture {
 public:
  // Block cannot be copy-assigned, that's why state is kept in a builder
  TestProposalBuilder complete_builder;

  void SetUp(benchmark::State &st) override {
    TestProposalBuilder builder;
    TestTransactionBuilder txbuilder;

    auto base_tx = txbuilder.createdTime(iroha::time::now()).quorum(1);

    for (int i = 0; i < number_of_commands; i++) {
      base_tx.transferAsset("player@one", "player@two", "coin", "", "5.00");
    }

    std::vector<shared_model::proto::Transaction> txs;

    for (int i = 0; i < number_of_txs; i++) {
      txs.push_back(base_tx.build());
    }

    complete_builder =
        builder.createdTime(iroha::time::now()).height(1).transactions(txs);
  }
};

/**
 * calls getters of a given object (block or proposal),
 * so that lazy fields are initialized.
 * @param obj - Block or Proposal
 */
template <typename T>
void checkLoop(const T &obj) {
  for (const auto &tx : obj.transactions()) {
    benchmark::DoNotOptimize(tx.commands());
  }
}

BENCHMARK_F(BlockBenchmark, CopyTest)(benchmark::State &st) {
  auto block = complete_builder.build();

  while (st.KeepRunning()) {
    shared_model::proto::Block copy(block.getTransport());

    checkLoop(copy);
  }
}

BENCHMARK_F(BlockBenchmark, MoveTest)(benchmark::State &st) {
  auto block = complete_builder.build();

  while (st.KeepRunning()) {
    shared_model::proto::Block copy(std::move(block.getTransport()));

    checkLoop(copy);
  }
}

BENCHMARK_F(BlockBenchmark, CloneTest)(benchmark::State &st) {
  auto block = complete_builder.build();

  while (st.KeepRunning()) {
    auto copy = clone(block);

    checkLoop(*copy);
  }
}

BENCHMARK_F(ProposalBenchmark, CopyTest)(benchmark::State &st) {
  auto proposal = complete_builder.build();

  while (st.KeepRunning()) {
    shared_model::proto::Proposal copy(proposal.getTransport());

    checkLoop(copy);
  }
}

BENCHMARK_F(ProposalBenchmark, MoveTest)(benchmark::State &state) {
  auto proposal = complete_builder.build();

  while (state.KeepRunning()) {
    shared_model::proto::Proposal copy(std::move(proposal.getTransport()));

    checkLoop(copy);
  }
}

BENCHMARK_F(ProposalBenchmark, CloneTest)(benchmark::State &st) {
  auto proposal = complete_builder.build();

  while (st.KeepRunning()) {
    auto copy = clone(proposal);

    checkLoop(*copy);
  }
}

BENCHMARK_MAIN();

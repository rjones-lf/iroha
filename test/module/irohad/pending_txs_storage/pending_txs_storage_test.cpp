/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "pending_txs_storage/impl/pending_txs_storage_impl.hpp"
#include <rxcpp/rx.hpp>
#include "module/irohad/pending_txs_storage/pending_txs_storage_fixture.hpp"

TEST_F(PendingTxsStorageFixture, FixutureSelfCheck) {
  auto state = std::make_shared<iroha::MstState>(iroha::MstState::empty());
  auto transactions =
      generateSharedBatch({{"alice@iroha", 2, 1}, {"bob@iroha", 2}});
  *state += transactions;
  ASSERT_EQ(state->getBatches().size(), 1) << "Failed to prepare MST state";
  ASSERT_EQ(state->getBatches().front()->transactions().size(), 2)
      << "Test batch contains wrong amount of transactions";
}

/**
 * Transactions insertion works in PendingTxsStorage
 * @given Batch of two transactions and storage
 * @when storage receives updated mst state with the batch
 * @then list of pending transactions can be received for all batch creators
 */
TEST_F(PendingTxsStorageFixture, InsertionTest) {
  auto state = std::make_shared<iroha::MstState>(iroha::MstState::empty());
  std::vector<TxData> source{{"alice@iroha", 2, 1}, {"bob@iroha", 2}};
  auto transactions = generateSharedBatch(source);
  *state += transactions;

  auto updates = rxcpp::observable<>::create<decltype(state)>([&state](auto s) {
    s.on_next(state);
    s.on_completed();
  });
  auto dummy = rxcpp::observable<>::create<std::shared_ptr<Batch>>(
      [](auto s) { s.on_completed(); });

  iroha::PendingTransactionStorageImpl storage(updates, dummy, dummy);
  for (const auto &creator : {"alice@iroha", "bob@iroha"}) {
    auto pending = storage.getPendingTransactions(creator);
    ASSERT_EQ(pending.size(), 2)
        << "Wrong amount of pending transactions was retrieved for " << creator
        << " account";

    // generally it's illegal way to verify the correctness.
    // here we can do it because the order is preserved by batch meta and there
    // are no transactions non-related to requested account
    for (auto i = 0u; i < pending.size(); ++i) {
      ASSERT_EQ(*pending[i], *(transactions->transactions()[i]));
    }
  }
}

/**
 * Updated batch replaces previously existed
 * @given Batch with one transaction with one signature and storage
 * @when transaction inside batch receives additional signature
 * @then pending transactions response is also updated
 */
TEST_F(PendingTxsStorageFixture, SignaturesUpdate) {
  auto state1 = std::make_shared<iroha::MstState>(iroha::MstState::empty());
  auto state2 = std::make_shared<iroha::MstState>(iroha::MstState::empty());
  std::vector<TxData> source = {{"alice@iroha", 3, 1}};
  auto transactions = generateSharedBatch(source);
  *state1 += transactions;
  source.front().addKeys(1);
  transactions = generateSharedBatch(source);
  *state2 += transactions;

  auto updates =
      rxcpp::observable<>::create<decltype(state1)>([&state1, &state2](auto s) {
        s.on_next(state1);
        s.on_next(state2);
        s.on_completed();
      });
  auto dummy = rxcpp::observable<>::create<std::shared_ptr<Batch>>(
      [](auto s) { s.on_completed(); });

  iroha::PendingTransactionStorageImpl storage(updates, dummy, dummy);
  auto pending = storage.getPendingTransactions("alice@iroha");
  ASSERT_EQ(pending.size(), 1);
  ASSERT_EQ(boost::size(pending.front()->signatures()), 2);
}

/**
 * Storage correctly handles storing of several batches
 * @given MST state update with three batches inside
 * @when different users asks pending transactions
 * @then users receives correct responses
 */
TEST_F(PendingTxsStorageFixture, SeveralBatches) {
  auto state = std::make_shared<iroha::MstState>(iroha::MstState::empty());
  auto batch1 = generateSharedBatch({{"alice@iroha", 2, 1}, {"bob@iroha", 2}});
  auto batch2 =
      generateSharedBatch({{"alice@iroha", 2, 1}, {"alice@iroha", 3}});
  auto batch3 = generateSharedBatch({{"bob@iroha", 2, 1}});
  *state += batch1;
  *state += batch2;
  *state += batch3;

  auto updates = rxcpp::observable<>::create<decltype(state)>([&state](auto s) {
    s.on_next(state);
    s.on_completed();
  });
  auto dummy = rxcpp::observable<>::create<std::shared_ptr<Batch>>(
      [](auto s) { s.on_completed(); });

  iroha::PendingTransactionStorageImpl storage(updates, dummy, dummy);
  auto alice_pending = storage.getPendingTransactions("alice@iroha");
  ASSERT_EQ(alice_pending.size(), 4);

  auto bob_pending = storage.getPendingTransactions("bob@iroha");
  ASSERT_EQ(bob_pending.size(), 3);
}

/**
 * New updates do not overwrite the whole state
 * @given two MST updates with different batches
 * @when updates arrives to storage sequentially
 * @then updates don't overwrite the whole storage state
 */
TEST_F(PendingTxsStorageFixture, SeparateBatchesDoNotOverwriteStorage) {
  auto state1 = std::make_shared<iroha::MstState>(iroha::MstState::empty());
  auto batch1 = generateSharedBatch({{"alice@iroha", 2, 1}, {"bob@iroha", 2}});
  *state1 += batch1;
  auto state2 = std::make_shared<iroha::MstState>(iroha::MstState::empty());
  auto batch2 =
      generateSharedBatch({{"alice@iroha", 2, 1}, {"alice@iroha", 3}});
  *state2 += batch2;

  auto updates =
      rxcpp::observable<>::create<decltype(state1)>([&state1, &state2](auto s) {
        s.on_next(state1);
        s.on_next(state2);
        s.on_completed();
      });
  auto dummy = rxcpp::observable<>::create<std::shared_ptr<Batch>>(
      [](auto s) { s.on_completed(); });

  iroha::PendingTransactionStorageImpl storage(updates, dummy, dummy);
  auto alice_pending = storage.getPendingTransactions("alice@iroha");
  ASSERT_EQ(alice_pending.size(), 4);

  auto bob_pending = storage.getPendingTransactions("bob@iroha");
  ASSERT_EQ(bob_pending.size(), 2);
}

/**
 * Batches with fully signed transactions (prepared transactions) should be
 * removed from storage
 * @given a batch with semi-signed transaction as MST update
 * @when the batch collects all the signatures
 * @then storage removes the batch
 */
TEST_F(PendingTxsStorageFixture, PreparedBatch) {
  auto state = std::make_shared<iroha::MstState>(iroha::MstState::empty());
  std::vector<TxData> source = {{"alice@iroha", 3, 1}};
  auto batch = generateSharedBatch(source);

  rxcpp::subjects::subject<decltype(batch)> prepared_batches_subject;
  auto updates = rxcpp::observable<>::create<decltype(state)>([&state](auto s) {
    s.on_next(state);
    s.on_completed();
  });
  auto dummy = rxcpp::observable<>::create<std::shared_ptr<Batch>>(
      [](auto s) { s.on_completed(); });
  iroha::PendingTransactionStorageImpl storage(
      updates, prepared_batches_subject.get_observable(), dummy);

  source.front().addKeys(2);
  batch = generateSharedBatch(source);
  prepared_batches_subject.get_subscriber().on_next(batch);
  prepared_batches_subject.get_subscriber().on_completed();
  auto pending = storage.getPendingTransactions("alice@iroha");
  ASSERT_EQ(pending.size(), 0);
}

/**
 * Batches with expired transactions should be removed from storage.
 * @given a batch with semi-signed transaction as MST update
 * @when the batch expires
 * @then storage removes the batch
 */
TEST_F(PendingTxsStorageFixture, ExpiredBatch) {
  auto state = std::make_shared<iroha::MstState>(iroha::MstState::empty());
  std::vector<TxData> source = {{"alice@iroha", 3, 1}};
  auto batch = generateSharedBatch(source);

  rxcpp::subjects::subject<decltype(batch)> expired_batches_subject;
  auto updates = rxcpp::observable<>::create<decltype(state)>([&state](auto s) {
    s.on_next(state);
    s.on_completed();
  });
  auto dummy = rxcpp::observable<>::create<std::shared_ptr<Batch>>(
      [](auto s) { s.on_completed(); });
  iroha::PendingTransactionStorageImpl storage(
      updates, dummy, expired_batches_subject.get_observable());

  expired_batches_subject.get_subscriber().on_next(batch);
  expired_batches_subject.get_subscriber().on_completed();
  auto pending = storage.getPendingTransactions("alice@iroha");
  ASSERT_EQ(pending.size(), 0);
}

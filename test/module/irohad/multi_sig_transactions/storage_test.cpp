/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>
#include <memory>
#include "logger/logger.hpp"
#include "module/irohad/multi_sig_transactions/mst_test_helpers.hpp"
#include "multi_sig_transactions/storage/mst_storage_impl.hpp"

auto log_ = logger::log("MstStorageTest");

using namespace iroha;

class StorageTestCompleter : public DefaultCompleter {
 public:
  bool operator()(const DataType &batch, const TimeType &time) const override {
    return std::all_of(
        batch->transactions().begin(),
        batch->transactions().end(),
        [&time](const auto &tx) { return tx->createdTime() < time; });
  }
};

class StorageTest : public testing::Test {
 public:
  StorageTest() : absent_peer(makePeer("localhost:50051", "absent")) {}

  void SetUp() override {
    storage = std::make_shared<MstStorageStateImpl>(
        std::make_shared<StorageTestCompleter>());
    fillOwnState();
  }

  void fillOwnState() {
    storage->updateOwnState(makeTestBatch(txBuilder(1, creation_time)));
    storage->updateOwnState(makeTestBatch(txBuilder(2, creation_time)));
    storage->updateOwnState(makeTestBatch(txBuilder(3, creation_time)));
  }

  std::shared_ptr<MstStorage> storage;
  std::shared_ptr<shared_model::interface::Peer> absent_peer;

  const unsigned quorum = 3u;
  const shared_model::interface::types::TimestampType creation_time =
      iroha::time::now();
};

TEST_F(StorageTest, StorageWhenApplyOtherState) {
  log_->info(
      "create state with default peers and other state => "
      "apply state");

  auto new_state = MstState::empty(std::make_shared<StorageTestCompleter>());
  new_state += makeTestBatch(txBuilder(5, creation_time));
  new_state += makeTestBatch(txBuilder(6, creation_time));
  new_state += makeTestBatch(txBuilder(7, creation_time));

  storage->apply(makePeer("localhost:50052", "another"), new_state);

  ASSERT_EQ(
      6, storage->getDiffState(absent_peer, creation_time).getBatches().size());
}

TEST_F(StorageTest, StorageInsertOtherState) {
  log_->info("init fixture state => get expired state");

  ASSERT_EQ(
      3,
      storage->getExpiredTransactions(creation_time + 1).getBatches().size());
  ASSERT_EQ(0,
            storage->getDiffState(absent_peer, creation_time + 1)
                .getBatches()
                .size());
}

TEST_F(StorageTest, StorageWhenCreateValidDiff) {
  log_->info("insert transactions => check their presence");

  ASSERT_EQ(
      3, storage->getDiffState(absent_peer, creation_time).getBatches().size());
}

TEST_F(StorageTest, StorageWhenCreate) {
  log_->info(
      "insert transactions => wait until expiring => "
      " check their absence");

  auto expiration_time = creation_time + 1;

  ASSERT_EQ(
      0,
      storage->getDiffState(absent_peer, expiration_time).getBatches().size());
}

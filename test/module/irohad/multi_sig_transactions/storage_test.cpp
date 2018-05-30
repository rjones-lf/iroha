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

#include <gtest/gtest.h>
#include <memory>
#include "logger/logger.hpp"
#include "module/irohad/multi_sig_transactions/mst_test_helpers.hpp"
#include "multi_sig_transactions/storage/mst_storage_impl.hpp"

auto log_ = logger::log("MstStorageTest");

using namespace iroha;

class StorageTestCompleter : public DefaultCompleter {
 public:
  bool operator()(const DataType &tx, const TimeType &time) const override {
    return tx->createdTime() < time;
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
    storage->updateOwnState(makeTx(1, creation_time));
    storage->updateOwnState(makeTx(2, creation_time));
    storage->updateOwnState(makeTx(3, creation_time));
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
  new_state += makeTx(5, creation_time);
  new_state += makeTx(6, creation_time);
  new_state += makeTx(7, creation_time);

  storage->apply(makePeer("localhost:50052", "another"), new_state);

  ASSERT_EQ(6,
            storage->getDiffState(absent_peer, creation_time)
                .getTransactions()
                .size());
}

TEST_F(StorageTest, StorageInsertOtherState) {
  log_->info("init fixture state => get expired state");

  ASSERT_EQ(3,
            storage->getExpiredTransactions(creation_time + 1)
                .getTransactions()
                .size());
  ASSERT_EQ(0,
            storage->getDiffState(absent_peer, creation_time + 1)
                .getTransactions()
                .size());
}

TEST_F(StorageTest, StorageWhenCreateValidDiff) {
  log_->info("insert transactions => check their presence");

  ASSERT_EQ(3,
            storage->getDiffState(absent_peer, creation_time)
                .getTransactions()
                .size());
}

TEST_F(StorageTest, StorageWhenCreate) {
  log_->info(
      "insert transactions => wait until expiring => "
      " check their absence");

  auto expiration_time = creation_time + 1;

  ASSERT_EQ(0,
            storage->getDiffState(absent_peer, expiration_time)
                .getTransactions()
                .size());
}

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
#include "logger/logger.hpp"
#include "multi_sig_transactions/storage/mst_storage_impl.hpp"
#include "module/irohad/multi_sig_transactions/mst_test_helpers.hpp"
#include <memory>

auto log_ = logger::log("MstStorageTest");

using namespace iroha;

class StorageTest : public testing::Test {
 public:

  void SetUp() override {
    my_peer = makePeer("1", "1");
    absent_peer = makePeer("absent", "absent");
    storage = make_shared<MstStorageStateImpl>(my_peer);
    fillOwnState();
  }

  void fillOwnState() {
    storage->updateOwnState(makeTx("1", "1"));
    storage->updateOwnState(makeTx("2", "2"));
    storage->updateOwnState(makeTx("3", "3"));
  }

  shared_ptr<MstStorage> storage;
  iroha::model::Peer my_peer;
  iroha::model::Peer absent_peer;
};

TEST_F(StorageTest, StorageInitialization) {
  log_->info("insert transactions => check their presence");

  ASSERT_EQ(3, storage->getDiffState(absent_peer).getTransactions().size());
}

TEST_F(StorageTest, StorageInsertOtherState) {
  log_->info("insert transactions for other peer as state => "
                 "check other state");
  MstState otherState;
  ((otherState += makeTx("3", "3"))
       += makeTx("4", "4"))
      += makeTx("5", "5");
  auto other_existed = makePeer("new", "new");
  storage->apply(other_existed, otherState);

  ASSERT_EQ(2, storage->getDiffState(other_existed).getTransactions().size());
  ASSERT_EQ(5, storage->getDiffState(absent_peer).getTransactions().size());
}

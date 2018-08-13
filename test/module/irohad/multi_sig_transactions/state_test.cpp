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
#include "module/irohad/multi_sig_transactions/mst_test_helpers.hpp"
#include "multi_sig_transactions/state/mst_state.hpp"

auto log_ = logger::log("MstStateTest");
using namespace std;
using namespace iroha;
using namespace iroha::model;

TEST(StateTest, CreateState) {
  log_->info("Create state from => insert one transaction");

  auto state = MstState::empty();
  ASSERT_EQ(0, state.getTransactions().size());
  state += makeTx(1);
  ASSERT_EQ(1, state.getTransactions().size());
}

TEST(StateTest, UpdateExistingState) {
  log_->info(
      "Create empty state => insert tx with one signature => "
      "insert tx with another signature");

  auto state = MstState::empty();
  auto time = iroha::time::now();
  state += makeTx(1, time);
  state += makeTx(1, time);
  ASSERT_EQ(1, state.getTransactions().size());
  ASSERT_EQ(2, boost::size(state.getTransactions().begin()->get()->signatures()));
}

TEST(StateTest, UpdateStateWhenTransacionsSame) {
  log_->info("Create empty state => insert two equal transaction");

  auto state = MstState::empty();

  auto keypair = makeKey();
  auto time = iroha::time::now();
  state += makeTx(1, time, keypair);
  state += makeTx(1, time, keypair);

  ASSERT_EQ(1, state.getTransactions().size());
  ASSERT_EQ(1, boost::size(state.getTransactions().begin()->get()->signatures()));
}

TEST(StateTest, DifferentSignaturesUnionTest) {
  log_->info("Create two states => merge them");

  auto state1 = MstState::empty();

  state1 += makeTx(1);
  state1 += makeTx(2);
  state1 += makeTx(3);

  ASSERT_EQ(3, state1.getTransactions().size());

  auto state2 = MstState::empty();
  state2 += makeTx(4);
  state2 += makeTx(5);
  ASSERT_EQ(2, state2.getTransactions().size());

  state1 += state2;
  ASSERT_EQ(5, state1.getTransactions().size());
}

TEST(StateTest, UnionStateWhenTransactionsSame) {
  log_->info("Create two states with common elements => merge them");

  auto time = iroha::time::now();
  auto keypair = makeKey();

  auto state1 = MstState::empty();
  state1 += makeTx(1, time, keypair);
  state1 += makeTx(2);

  ASSERT_EQ(2, state1.getTransactions().size());

  auto state2 = MstState::empty();
  state2 += makeTx(1, time, keypair);
  state2 += makeTx(5);
  ASSERT_EQ(2, state2.getTransactions().size());

  state1 += state2;
  ASSERT_EQ(3, state1.getTransactions().size());
}

TEST(StateTest, UnionStateWhenSameTransactionHaveDifferentSignatures) {
  log_->info(
      "Create two transactions with different signatures => move them"
      " into owns states => merge states");

  auto time = iroha::time::now();

  auto state1 = MstState::empty();
  auto state2 = MstState::empty();

  state1 += makeTx(1, time, makeKey());
  state2 += makeTx(1, time, makeKey());

  state1 += state2;
  ASSERT_EQ(1, state1.getTransactions().size());
  ASSERT_EQ(2, boost::size(state1.getTransactions().begin()->get()->signatures()));
}

TEST(StateTest, DifferenceTest) {
  log_->info("Create two sets with common element => perform diff operation");

  auto keypair = makeKey();
  auto time = iroha::time::now();

  auto state1 = MstState::empty();
  auto state2 = MstState::empty();
  state1 += makeTx(1);
  state1 += makeTx(2, time, keypair);

  state2 += makeTx(2, time, keypair);
  state2 += makeTx(3);

  MstState diff = state1 - state2;
  ASSERT_EQ(1, diff.getTransactions().size());
}

TEST(StateTest, UpdateTxUntillQuorum) {
  log_->info("Update transaction signature until quorum happens");

  auto quorum = 3u;
  auto time = iroha::time::now();

  auto state = MstState::empty();

  auto state_after_one_tx = state += makeTx(1, time, makeKey(), quorum);
  ASSERT_EQ(0, state_after_one_tx.getTransactions().size());

  auto state_after_two_txes = state += makeTx(1, time, makeKey(), quorum);
  ASSERT_EQ(0, state_after_one_tx.getTransactions().size());

  auto state_after_three_txes = state += makeTx(1, time, makeKey(), quorum);
  ASSERT_EQ(1, state_after_three_txes.getTransactions().size());
  ASSERT_EQ(0, state.getTransactions().size());
}

TEST(StateTest, UpdateStateWithNewStateUntilQuorum) {
  log_->info("Merge two states that contains common transaction");

  auto quorum = 3u;
  auto keypair = makeKey();
  auto time = iroha::time::now();

  auto state1 = MstState::empty();
  state1 += makeTx(1, time, makeKey(), quorum);
  state1 += makeTx(1, time, keypair, quorum);
  state1 += makeTx(2, time, makeKey(), quorum);
  ASSERT_EQ(2, state1.getTransactions().size());

  auto state2 = MstState::empty();
  state2 += makeTx(1, time, keypair, quorum);
  state2 += makeTx(1, time, makeKey(), quorum);
  ASSERT_EQ(1, state2.getTransactions().size());

  auto completed_state = state1 += state2;
  ASSERT_EQ(1, completed_state.getTransactions().size());
  ASSERT_EQ(1, state1.getTransactions().size());
}

class TimeTestCompleter : public iroha::DefaultCompleter {
  bool operator()(const DataType &tx, const TimeType &time) const override {
    return tx->createdTime() < time;
  }
};

TEST(StateTest, TimeIndexInsertionByTx) {
  log_->info("Insert one transaction with many signatures => erase tx by time");

  auto quorum = 3u;
  auto time = iroha::time::now();

  auto state = MstState::empty(std::make_shared<TimeTestCompleter>());

  state += makeTx(1, time, makeKey(), quorum);
  state += makeTx(1, time, makeKey(), quorum);

  ASSERT_EQ(1, state.getTransactions().size());

  auto expired_state = state.eraseByTime(time + 1);
  ASSERT_EQ(1, expired_state.getTransactions().size());
  ASSERT_EQ(0, state.getTransactions().size());
}

TEST(StateTest, TimeIndexInsertionByAddState) {
  log_->info("Fill two states => add one to another => erase tx by time");

  auto quorum = 3u;
  auto time = iroha::time::now();

  auto state1 = MstState::empty(std::make_shared<TimeTestCompleter>());
  state1 += makeTx(1, time, makeKey(), quorum);
  state1 += makeTx(1, time, makeKey(), quorum);

  auto state2 = MstState::empty(std::make_shared<TimeTestCompleter>());
  state2 += makeTx(5, time, makeKey(), quorum);
  state2 += makeTx(6, time, makeKey(), quorum);

  auto completed_state = state1 += state2;
  ASSERT_EQ(0, completed_state.getTransactions().size());

  auto expired_state = state1.eraseByTime(time + 1);
  ASSERT_EQ(3, expired_state.getTransactions().size());
  ASSERT_EQ(0, state1.getTransactions().size());
  ASSERT_EQ(2, state2.getTransactions().size());
}

TEST(StateTest, RemovingTestWhenByTimeExpired) {
  log_->info(
      "Create one filled state and one empty => "
      "remove second from first "
      "=> perform time expiration method");

  auto quorum = 3u;
  auto time = iroha::time::now();

  auto state1 = MstState::empty(std::make_shared<TimeTestCompleter>());
  state1 += makeTx(1, time, makeKey(), quorum);
  state1 += makeTx(2, time, makeKey(), quorum);

  auto state2 = MstState::empty(std::make_shared<TimeTestCompleter>());

  auto diff_state = state1 - state2;

  ASSERT_EQ(2, diff_state.getTransactions().size());

  auto expired_state = diff_state.eraseByTime(time + 1);
  ASSERT_EQ(0, diff_state.getTransactions().size());
  ASSERT_EQ(2, expired_state.getTransactions().size());
}

/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>
#include "logger/logger.hpp"
#include "module/irohad/multi_sig_transactions/mst_test_helpers.hpp"
#include "multi_sig_transactions/state/mst_state.hpp"

auto log_ = logger::log("MstStateTest");
using namespace std;
using namespace iroha;
using namespace iroha::model;

/**
 * @given empty state
 * @when  insert one batch
 * @then  checks that batch is holded by state
 */
TEST(StateTest, CreateState) {
  auto state = MstState::empty();
  ASSERT_EQ(0, state.getBatches().size());
  log_->info("first check");
  state += addSignatures(
      makeTestBatch(txBuilder(1)), 0, makeSignature("1", "pub_key_1"));
  log_->info("add");
  ASSERT_EQ(1, state.getBatches().size());
}

/**
 * @given empty state
 * @when  insert batches with different signatures
 * @then  checks that signatures are merged into the state
 */
TEST(StateTest, UpdateExistingState) {
  auto state = MstState::empty();
  auto time = iroha::time::now();
  state += addSignatures(
      makeTestBatch(txBuilder(1, time)), 0, makeSignature("1", "pub_key_1"));

  state += addSignatures(
      makeTestBatch(txBuilder(1, time)), 0, makeSignature("2", "pub_key_2"));
  ASSERT_EQ(1, state.getBatches().size());
  ASSERT_EQ(1, state.getBatches().begin()->get()->transactions().size());
  ASSERT_EQ(2,
            boost::size(state.getBatches()
                            .begin()
                            ->get()
                            ->transactions()
                            .begin()
                            ->get()
                            ->signatures()));
}

/**
 * @given empty state
 * @when  insert batch with same signatures two times
 * @then  checks that appears only one signature
 */
TEST(StateTest, UpdateStateWhenTransacionsSame) {
  log_->info("Create empty state => insert two equal transaction");

  auto state = MstState::empty();

  auto time = iroha::time::now();
  state += addSignatures(
      makeTestBatch(txBuilder(1, time)), 0, makeSignature("1", "1"));
  state += addSignatures(
      makeTestBatch(txBuilder(1, time)), 0, makeSignature("1", "1"));

  ASSERT_EQ(1, state.getBatches().size());
  ASSERT_EQ(1,
            boost::size(state.getBatches()
                            .begin()
                            ->get()
                            ->transactions()
                            .begin()
                            ->get()
                            ->signatures()));
}

/**
 * @given prepared state with 3 batches
 * @when  insert independent state
 * @then  checks that all batches are here
 */
TEST(StateTest, DifferentSignaturesUnionTest) {
  log_->info("Create two states => merge them");

  auto state1 = MstState::empty();

  state1 +=
      addSignatures(makeTestBatch(txBuilder(1)), 0, makeSignature("1", "1"));

  state1 +=
      addSignatures(makeTestBatch(txBuilder(2)), 0, makeSignature("2", "2"));
  state1 +=
      addSignatures(makeTestBatch(txBuilder(3)), 0, makeSignature("3", "3"));

  ASSERT_EQ(3, state1.getBatches().size());

  auto state2 = MstState::empty();
  state2 +=
      addSignatures(makeTestBatch(txBuilder(4)), 0, makeSignature("4", "4"));
  state2 +=
      addSignatures(makeTestBatch(txBuilder(5)), 0, makeSignature("5", "5"));
  ASSERT_EQ(2, state2.getBatches().size());

  state1 += state2;
  ASSERT_EQ(5, state1.getBatches().size());
}

/**
 * @given two empty states
 * @when insert transaction with quorum 2 to one state
 * AND insert same transaction with another signature to second state
 * AND merge states
 * @then check that merged state contains both signatures
 */
TEST(StateTest, UnionStateWhenSameTransactionHaveDifferentSignatures) {
  log_->info(
      "Create two transactions with different signatures => move them"
      " into owns states => merge states");

  auto time = iroha::time::now();

  auto state1 = MstState::empty();
  auto state2 = MstState::empty();

  state1 += addSignatures(
      makeTestBatch(txBuilder(1, time)), 0, makeSignature("1", "1"));
  state2 += addSignatures(
      makeTestBatch(txBuilder(1, time)), 0, makeSignature("2", "2"));

  state1 += state2;
  ASSERT_EQ(1, state1.getBatches().size());
  ASSERT_EQ(2,
            boost::size(state1.getBatches()
                            .begin()
                            ->get()
                            ->transactions()
                            .begin()
                            ->get()
                            ->signatures()));
}

/**
 * @given prepared state with two batches
 * AND    another state with tx and signature
 * @when  merge states
 * @then  checks that final state collapse same tx
 */
TEST(StateTest, UnionStateWhenTransactionsSame) {
  auto time = iroha::time::now();

  auto state1 = MstState::empty();
  state1 += addSignatures(
      makeTestBatch(txBuilder(1, time)), 0, makeSignature("1", "1"));
  state1 += addSignatures(
      makeTestBatch(txBuilder(2)), 0, makeSignature("other", "other"));

  ASSERT_EQ(2, state1.getBatches().size());

  auto state2 = MstState::empty();
  state2 += addSignatures(
      makeTestBatch(txBuilder(1, time)), 0, makeSignature("1", "1"));
  state2 += addSignatures(
      makeTestBatch(txBuilder(3)), 0, makeSignature("other_", "other_"));
  ASSERT_EQ(2, state2.getBatches().size());

  state1 += state2;
  ASSERT_EQ(3, state1.getBatches().size());
}

/**
 * @given two states with common element
 * @when  performs diff operaton for states
 * @then  check that common element is presented
 */
TEST(StateTest, DifferenceTest) {
  auto time = iroha::time::now();

  auto state1 = MstState::empty();
  state1 += addSignatures(
      makeTestBatch(txBuilder(1, time)), 0, makeSignature("1", "1"));
  state1 +=
      addSignatures(makeTestBatch(txBuilder(2)), 0, makeSignature("2", "2"));

  auto state2 = MstState::empty();
  state2 += addSignatures(
      makeTestBatch(txBuilder(1, time)), 0, makeSignature("2_2", "2_2"));
  state2 +=
      addSignatures(makeTestBatch(txBuilder(3)), 0, makeSignature("3", "3"));

  MstState diff = state1 - state2;
  ASSERT_EQ(1, diff.getBatches().size());
}

/**
 * @given empty state
 * @when insert transaction with quorum 3, 3 times
 * @then check that transaction is compelete
 */
TEST(StateTest, UpdateTxUntillQuorum) {
  auto quorum = 3u;
  auto time = iroha::time::now();

  auto state = MstState::empty();

  auto state_after_one_tx = state += addSignatures(
      makeTestBatch(txBuilder(1, time, quorum)), 0, makeSignature("1", "1"));
  ASSERT_EQ(0, state_after_one_tx.getBatches().size());

  auto state_after_two_txes = state += addSignatures(
      makeTestBatch(txBuilder(1, time, quorum)), 0, makeSignature("2", "2"));
  ASSERT_EQ(0, state_after_one_tx.getBatches().size());

  auto state_after_three_txes = state += addSignatures(
      makeTestBatch(txBuilder(1, time, quorum)), 0, makeSignature("3", "3"));
  ASSERT_EQ(1, state_after_three_txes.getBatches().size());
  ASSERT_EQ(0, state.getBatches().size());
}

/**
 * @given two states with same transaction, where transaction will complete
 * after merge
 * @when  merge states
 * @then  checks that transaction go out to completed state
 */
TEST(StateTest, UpdateStateWithNewStateUntilQuorum) {
  auto quorum = 3u;
  auto keypair = makeKey();
  auto time = iroha::time::now();

  auto state1 = MstState::empty();
  state1 += addSignatures(makeTestBatch(txBuilder(1, time, quorum)),
                          0,
                          makeSignature("1_1", "1_1"));
  state1 += addSignatures(
      makeTestBatch(txBuilder(2, time)), 0, makeSignature("2", "2"));
  state1 += addSignatures(
      makeTestBatch(txBuilder(2, time)), 0, makeSignature("3", "3"));
  ASSERT_EQ(2, state1.getBatches().size());

  auto state2 = MstState::empty();
  state2 += addSignatures(makeTestBatch(txBuilder(1, time, quorum)),
                          0,
                          makeSignature("1_2", "1_2"));
  state2 += addSignatures(makeTestBatch(txBuilder(1, time, quorum)),
                          0,
                          makeSignature("1_3", "1_3"));
  ASSERT_EQ(1, state2.getBatches().size());

  auto completed_state = state1 += state2;
  ASSERT_EQ(1, completed_state.getBatches().size());
  ASSERT_EQ(1, state1.getBatches().size());
}

/**
 * Tests expired completer, which checks that all transactions in batch are not
 * expired
 */
class TimeTestCompleter : public iroha::DefaultCompleter {
  bool operator()(const DataType &batch, const TimeType &time) const override {
    return std::all_of(
        batch->transactions().begin(),
        batch->transactions().end(),
        [&time](const auto &tx) { return tx->createdTime() < time; });
  }
};

/**
 * @given state with one transaction
 * @when  call erase by time, where batch will be already expired
 * @then  checks that expired state contains batch and initial doesn't conitain
 */
TEST(StateTest, TimeIndexInsertionByTx) {
  auto quorum = 2u;
  auto time = iroha::time::now();

  auto state = MstState::empty(std::make_shared<TimeTestCompleter>());

  state += addSignatures(makeTestBatch(txBuilder(1, time, quorum)),
                         0,
                         makeSignature("1_1", "1_1"));

  auto expired_state = state.eraseByTime(time + 1);
  ASSERT_EQ(1, expired_state.getBatches().size());
  ASSERT_EQ(0, state.getBatches().size());
}

/**
 * @given init two states
 * @when  merge them
 * AND make states expired
 * @then checks that all expired transactions are preserved in expired state
 */
TEST(StateTest, TimeIndexInsertionByAddState) {
  auto quorum = 3u;
  auto time = iroha::time::now();

  auto state1 = MstState::empty(std::make_shared<TimeTestCompleter>());
  state1 += addSignatures(makeTestBatch(txBuilder(1, time, quorum)),
                          0,
                          makeSignature("1_1", "1_1"));
  state1 += addSignatures(makeTestBatch(txBuilder(1, time, quorum)),
                          0,
                          makeSignature("1_2", "1_2"));

  auto state2 = MstState::empty(std::make_shared<TimeTestCompleter>());
  state2 += addSignatures(
      makeTestBatch(txBuilder(2, time)), 0, makeSignature("2", "2"));
  state2 += addSignatures(
      makeTestBatch(txBuilder(3, time)), 0, makeSignature("3", "3"));

  auto completed_state = state1 += state2;
  ASSERT_EQ(0, completed_state.getBatches().size());

  auto expired_state = state1.eraseByTime(time + 1);
  ASSERT_EQ(3, expired_state.getBatches().size());
  ASSERT_EQ(0, state1.getBatches().size());
  ASSERT_EQ(2, state2.getBatches().size());
}

/**
 * @given first state with two batches, seconds is empty
 * @when  remove second from first
 * AND call erase by time in diff state
 * @then checks that expired state contains batches and diff is not
 */
TEST(StateTest, RemovingTestWhenByTimeExpired) {
  auto time = iroha::time::now();

  auto state1 = MstState::empty(std::make_shared<TimeTestCompleter>());
  state1 += addSignatures(
      makeTestBatch(txBuilder(1, time)), 0, makeSignature("2", "2"));
  state1 += addSignatures(
      makeTestBatch(txBuilder(2, time)), 0, makeSignature("2", "2"));

  auto state2 = MstState::empty(std::make_shared<TimeTestCompleter>());

  auto diff_state = state1 - state2;

  ASSERT_EQ(2, diff_state.getBatches().size());

  auto expired_state = diff_state.eraseByTime(time + 1);
  ASSERT_EQ(2, expired_state.getBatches().size());
  ASSERT_EQ(0, diff_state.getBatches().size());
}

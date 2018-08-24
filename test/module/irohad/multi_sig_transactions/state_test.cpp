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
 * @then  checks that state contains the inserted batch
 */
TEST(StateTest, CreateState) {
  auto state = MstState::empty();
  ASSERT_EQ(0, state.getBatches().size());
  auto tx = addSignatures(
      makeTestBatch(txBuilder(1)), 0, makeSignature("1", "pub_key_1"));
  state += tx;
  ASSERT_EQ(1, state.getBatches().size());
  ASSERT_EQ(*tx, *state.getBatches().at(0));
}

/**
 * @given empty state
 * @when  insert batches with different signatures
 * @then  checks that signatures are merged into the state
 */
TEST(StateTest, UpdateExistingState) {
  auto state = MstState::empty();
  auto time = iroha::time::now();

  auto first_signature = makeSignature("1", "pub_key_1");
  auto second_signature = makeSignature("2", "pub_key_2");
  auto base_tx = makeTestBatch(txBuilder(1, time));

  auto first_tx = addSignatures(base_tx, 0, first_signature);
  state += first_tx;

  auto second_tx = addSignatures(base_tx, 0, second_signature);
  state += second_tx;
  ASSERT_EQ(1, state.getBatches().size());

  auto merged_tx = addSignatures(base_tx, 0, first_signature, second_signature);
  ASSERT_EQ(*merged_tx, *state.getBatches().at(0));
}

/**
 * @given empty state
 * @when  insert batch with same signatures two times
 * @then  checks that the state contains only one signature
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
 * @when  insert the batches, which are not present in the state
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
 * @given given two states with one common transaction but with different
 * signatures
 * @when  states are merged
 * @then  the common transaction is not appeared in the resulting state
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
 * @given two states with a common element
 * @when  difference of the states is taken
 * @then  the common element is present in the difference set
 */
TEST(StateTest, DifferenceTest) {
  auto time = iroha::time::now();

  auto first_signature = makeSignature("1", "1");
  auto second_signature = makeSignature("2", "2");
  auto third_signature = makeSignature("3", "3");

  auto another_signature = makeSignature("another", "another");

  auto common_batch = makeTestBatch(txBuilder(1, time));
  auto another_batch = makeTestBatch(txBuilder(3));

  auto state1 = MstState::empty();
  state1 += addSignatures(common_batch, 0, first_signature);
  state1 += addSignatures(common_batch, 0, second_signature);

  auto state2 = MstState::empty();
  state2 += addSignatures(common_batch, 0, second_signature);
  state2 += addSignatures(common_batch, 0, third_signature);
  state2 += addSignatures(another_batch, 0, another_signature);

  MstState diff = state1 - state2;
  ASSERT_EQ(1, diff.getBatches().size());
  auto expected_batch = addSignatures(common_batch, 0, first_signature);
  ASSERT_EQ(*expected_batch, *diff.getBatches().at(0));
}

/**
 * @given an empty state
 * @when a partially signed transaction with quorum 3 is inserted 3 times
 * @then  the resulting state contains one signed transaction
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
 * @given two states with one common partially signed transaction
 * @when  the states are merged
 * @then  the resulting state contains one signed transaction
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
 * @given a timepoint
 * AND    a state with an expired transaction
 * @when  erase by time is called
 * @then   the resulting state contains the expired transaction
 */
TEST(StateTest, TimeIndexInsertionByTx) {
  auto quorum = 2u;
  auto time = iroha::time::now();

  auto prepared_batch = addSignatures(makeTestBatch(txBuilder(1, time, quorum)),
                                      0,
                                      makeSignature("1_1", "1_1"));

  auto state = MstState::empty(std::make_shared<TimeTestCompleter>());

  state += prepared_batch;

  auto expired_state = state.eraseByTime(time + 1);
  ASSERT_EQ(1, expired_state.getBatches().size());
  ASSERT_EQ(*prepared_batch, *expired_state.getBatches().at(0));
  ASSERT_EQ(0, state.getBatches().size());
}

/**
 * @given two states with expired transactions
 * @when  merge them
 * AND    make states expired
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
}

/**
 * @given a state with two batches
 * AND    an empty state
 * @when  the second state is removed from the first state
 * AND  erase by time is called
 * @then checks that the resulting state does not contain any transactions
 */
TEST(StateTest, RemovingTestWhenByTimeHasExpired) {
  auto time = iroha::time::now();

  auto state1 = MstState::empty(std::make_shared<TimeTestCompleter>());
  state1 += addSignatures(
      makeTestBatch(txBuilder(1, time)), 0, makeSignature("2", "2"));
  state1 += addSignatures(
      makeTestBatch(txBuilder(2, time)), 0, makeSignature("2", "2"));

  auto state2 = MstState::empty(std::make_shared<TimeTestCompleter>());

  auto diff_state = state1 - state2;

  ASSERT_EQ(2, diff_state.getBatches().size());
}

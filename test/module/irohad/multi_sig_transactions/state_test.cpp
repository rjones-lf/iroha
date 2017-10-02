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
#include "multi_sig_transactions/storage/mst_state.hpp"
#include <string>
#include "common/types.hpp"

auto log_ = logger::log("MstStateTest");
using namespace std;
using namespace iroha;
using namespace iroha::model;

auto generateSignature(const string &sign_value) {
  Signature s;
  s.signature = stringToBytesFiller<Signature::SignatureType>(sign_value);
  s.pubkey = stringToBytesFiller<Signature::KeyType>(sign_value);
  log_->info("Signature({}, {})",
             s.signature.to_string(),
             s.pubkey.to_string());
  return s;
}

auto makeTx(const string &hash_value, const string &signature_value) {
  Transaction tx{};
  tx.tx_hash = stringToBytesFiller<Transaction::HashType>(hash_value);
  tx.signatures = {generateSignature(signature_value)};
  return make_shared<Transaction>(tx);
}

TEST(StateTest, UpdateState) {
  log_->info("Create empty state => insert one transaction");

  MstState state;
  ASSERT_EQ(0, state.getTransactions().size());
  state += makeTx("1", "1");
  ASSERT_EQ(1, state.getTransactions().size());
}

TEST(StateTest, UpdateExistingState) {
  log_->info("Create empty state => insert tx with one signature => "
                 "insert tx with another signature");

  MstState state;
  state += makeTx("1", "1");
  state += makeTx("1", "2");
  ASSERT_EQ(1, state.getTransactions().size());
  ASSERT_EQ(2, state.getTransactions().begin()->get()->signatures.size());
}

TEST(StateTest, UpdateStateWhenTransacionsSame) {
  log_->info("Create empty state => insert two equal transaction");

  MstState state;

  state += makeTx("1", "1");

  state += makeTx("1", "1");

  ASSERT_EQ(1, state.getTransactions().size());
}

TEST(StateTest, DifferentSignaturesUnionTest) {
  log_->info("Create two states => merge them");

  MstState state1;
  ((state1 += makeTx("1", "1"))
       += makeTx("2", "2"))
      += makeTx("3", "3");

  ASSERT_EQ(3, state1.getTransactions().size());

  MstState state2;
  (state2 += makeTx("4", "4"))
      += makeTx("5", "5");
  ASSERT_EQ(2, state2.getTransactions().size());

  auto union_state = state1 + state2;
  ASSERT_EQ(5, union_state.getTransactions().size());
}

TEST(StateTest, UnionStateWhenTransactionsSame) {
  log_->info("Create two states with common elements => merge them");

  MstState state1;
  (state1 += makeTx("1", "1"))
      += makeTx("2", "2");

  ASSERT_EQ(2, state1.getTransactions().size());

  MstState state2;
  (state2 += makeTx("1", "1"))
      += makeTx("5", "5");
  ASSERT_EQ(2, state2.getTransactions().size());

  auto union_state = state1 + state2;
  ASSERT_EQ(3, union_state.getTransactions().size());
}

TEST(StateTest, UnionStateWhenSameTransactionHaveDifferentSignatures) {
  log_->info("Create two transactions with different signatures => move them"
                 " into owns states => merge states");

  MstState state1, state2;
  state1 += makeTx("1", "1");
  state2 += makeTx("1", "2");

  auto union_state = state1 + state2;
  ASSERT_EQ(1, union_state.getTransactions().size());
  ASSERT_EQ(2, union_state.getTransactions().begin()->get()->signatures.size());
}

TEST(StateTest, DifferenceTest){
  log_->info("Create two sets with common element => perform diff operation");

  MstState state1, state2;
  (state1 += makeTx("1", "1"))
      += makeTx("2", "2");

  (state2 += makeTx("2", "2"))
      += makeTx("3", "3");

  MstState diff = state1 - state2;
  ASSERT_EQ(1, diff.getTransactions().size());
}


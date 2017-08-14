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
#include <algorithm>
#include "consensus/yac/storage/yac_vote_storage.hpp"
#include "yac_mocks.hpp"

#include <iostream>
using namespace std;

using namespace iroha::consensus::yac;

using pub_t = iroha::ed25519::pubkey_t;

TEST(YacStorageTest, SupermajorityFunctionForAllCases2) {
  cout << "-----------| F(x, 2), x in {0..3} -----------" << endl;

  int N = 2;
  ASSERT_FALSE(hasSupermajority(0, N));
  ASSERT_FALSE(hasSupermajority(1, N));
  ASSERT_TRUE(hasSupermajority(2, N));
  ASSERT_FALSE(hasSupermajority(3, N));
}

TEST(YacStorageTest, SupermajorityFunctionForAllCases4) {
  cout << "-----------| F(x, 4), x in {0..5} |-----------" << endl;

  int N = 4;
  ASSERT_FALSE(hasSupermajority(0, N));
  ASSERT_FALSE(hasSupermajority(1, N));
  ASSERT_FALSE(hasSupermajority(2, N));
  ASSERT_TRUE(hasSupermajority(3, N));
  ASSERT_TRUE(hasSupermajority(4, N));
  ASSERT_FALSE(hasSupermajority(5, N));
}

TEST(YacStorageTest, YacBlockStorageWhenNormalDataInput) {
  cout << "-----------| Sequentially insertion of votes |-----------" << endl;

  YacHash hash("proposal", "commit");
  int N = 4;
  YacBlockStorage storage(hash, N);

  auto insert_1 = storage.insert(
      create_vote(hash, pub_t::from_string_var("one").to_string()));
  ASSERT_EQ(CommitState::not_committed, insert_1.state);
  ASSERT_EQ(nonstd::nullopt, insert_1.answer.commit);
  ASSERT_EQ(nonstd::nullopt, insert_1.answer.reject);

  auto insert_2 = storage.insert(
      create_vote(hash, pub_t::from_string_var("two").to_string()));
  ASSERT_EQ(CommitState::not_committed, insert_2.state);
  ASSERT_EQ(nonstd::nullopt, insert_2.answer.commit);
  ASSERT_EQ(nonstd::nullopt, insert_2.answer.reject);

  auto insert_3 = storage.insert(
      create_vote(hash, pub_t::from_string_var("three").to_string()));
  ASSERT_EQ(CommitState::committed, insert_3.state);
  ASSERT_NE(nonstd::nullopt, insert_3.answer.commit);
  ASSERT_EQ(3, insert_3.answer.commit->votes.size());
  ASSERT_EQ(nonstd::nullopt, insert_3.answer.reject);

  auto insert_4 = storage.insert(
      create_vote(hash, pub_t::from_string_var("four").to_string()));
  ASSERT_EQ(CommitState::committed_before, insert_4.state);
  ASSERT_EQ(4, insert_4.answer.commit->votes.size());
  ASSERT_EQ(nonstd::nullopt, insert_4.answer.reject);
}

TEST(YacStorageTest, YacBlockStorageWhenNotCommittedAndCommitAcheive) {
  cout << "-----------| Insert vote => insert commit |-----------" << endl;

  YacHash hash("proposal", "commit");
  int N = 4;
  YacBlockStorage storage(hash, N);

  auto insert_1 = storage.insert(
      create_vote(hash, pub_t::from_string_var("one").to_string()));
  ASSERT_EQ(CommitState::not_committed, insert_1.state);
  ASSERT_EQ(nonstd::nullopt, insert_1.answer.commit);
  ASSERT_EQ(nonstd::nullopt, insert_1.answer.reject);

  auto insert_commit = storage.insert(CommitMessage(
      {create_vote(hash, pub_t::from_string_var("two").to_string()),
       create_vote(hash, pub_t::from_string_var("three").to_string()),
       create_vote(hash, pub_t::from_string_var("four").to_string())}));
  ASSERT_EQ(CommitState::committed, insert_commit.state);
  ASSERT_EQ(4, insert_commit.answer.commit->votes.size());
  ASSERT_EQ(nonstd::nullopt, insert_1.answer.reject);
}

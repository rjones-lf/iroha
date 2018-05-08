/**
 * Copyright Soramitsu Co., Ltd. 2018 All Rights Reserved.
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

#include "backend/protobuf/common_objects/signature.hpp"
#include "builders/protobuf/common_objects/proto_peer_builder.hpp"
#include "builders/protobuf/common_objects/proto_signature_builder.hpp"
#include "consensus/yac/impl/supermajority_checker_impl.hpp"
#include "logger/logger.hpp"
#include "module/shared_model/builders/protobuf/test_block_builder.hpp"

using namespace iroha::consensus::yac;

static logger::Logger log_ = logger::testLog("YacCommon");

class SupermajorityCheckerTest : public ::testing::Test,
                                 public SupermajorityCheckerImpl {
 public:
  void SetUp() override {}
};

/**
 * @given 2 participants
 * @when check range of voted participants
 * @then correct result
 */
TEST_F(SupermajorityCheckerTest, SuperMajorityCheckWithSize2) {
  log_->info("-----------| F(x, 2), x in {0..3} -----------");

  int N = 2;
  ASSERT_FALSE(checkSize(0, N));
  ASSERT_FALSE(checkSize(1, N));
  ASSERT_TRUE(checkSize(2, N));
  ASSERT_FALSE(checkSize(3, N));
}

/**
 * @given 4 participants
 * @when check range of voted participants
 * @then correct result
 */
TEST_F(SupermajorityCheckerTest, SuperMajorityCheckWithSize4) {
  log_->info("-----------| F(x, 4), x in {0..5} |-----------");

  int N = 4;
  ASSERT_FALSE(checkSize(0, N));
  ASSERT_FALSE(checkSize(1, N));
  ASSERT_FALSE(checkSize(2, N));
  ASSERT_TRUE(checkSize(3, N));
  ASSERT_TRUE(checkSize(4, N));
  ASSERT_FALSE(checkSize(5, N));
}

/**
 * @given 7 participants, 6 voted
 * @when check range of frequent elements
 * @then correct result
 */
TEST_F(SupermajorityCheckerTest, RejectProofSuccessfulCase) {
  log_->info("-----------| RejectProof(x, 6, 7) in {1..3} |-----------");

  ASSERT_TRUE(hasReject(1, 6, 7));
  ASSERT_TRUE(hasReject(2, 6, 7));
  ASSERT_TRUE(hasReject(3, 6, 7));
}

/**
 * @given 7 participants, 6 voted
 * @when check range of frequent elements
 * @then correct result
 */
TEST_F(SupermajorityCheckerTest, RejectProofNegativeCase) {
  log_->info("-----------| RejectProof(x, 6, 7) in {4..6}|-----------");

  ASSERT_FALSE(hasReject(4, 6, 7));
  ASSERT_FALSE(hasReject(5, 6, 7));
  ASSERT_FALSE(hasReject(6, 6, 7));
}

/**
 * @given a pair of peers and a pair different signatures by the first peer
 * @when hasSupermajority is called
 * @then it return false
 */
TEST_F(SupermajorityCheckerTest, PublicKeyUniqueness) {
  using namespace shared_model::crypto;
  std::vector<std::shared_ptr<shared_model::interface::Peer>> peers;
  auto make_peer_key = [&peers](const std::string &key) {
    PublicKey pub_key(key);
    peers.emplace_back(clone(shared_model::proto::PeerBuilder()
                                 .address("localhost")
                                 .pubkey(pub_key)
                                 .build()));
    return pub_key;
  };

  auto peer_key = make_peer_key(std::string(32, '0'));
  make_peer_key(std::string(32, '1'));

  auto block = TestBlockBuilder().build();
  block.addSignature(Signed("1"), peer_key);
  block.addSignature(Signed("2"), peer_key);

  ASSERT_FALSE(hasSupermajority(block.signatures(), peers));
}

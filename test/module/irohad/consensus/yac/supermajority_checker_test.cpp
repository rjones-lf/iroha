/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "consensus/yac/impl/supermajority_checker_impl.hpp"

#include <gtest/gtest.h>

#include <boost/range/adaptor/indirected.hpp>
#include "logger/logger.hpp"

#include "module/shared_model/interface_mocks.hpp"

using namespace iroha::consensus::yac;

using ::testing::ReturnRefOfCopy;

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
  using namespace std::string_literals;
  std::vector<std::shared_ptr<shared_model::interface::Peer>> peers;
  auto make_peer_key = [&peers](const std::string &key) {
    PublicKey pub_key(key);
    auto peer = std::make_shared<MockPeer>();
    EXPECT_CALL(*peer, pubkey()).WillRepeatedly(ReturnRefOfCopy(pub_key));

    peers.push_back(peer);
    return pub_key;
  };

  auto peer_key = make_peer_key(std::string(32, '0'));
  make_peer_key(std::string(32, '1'));

  auto sig = std::make_shared<MockSignature>();
  EXPECT_CALL(*sig, publicKey()).WillRepeatedly(ReturnRefOfCopy(peer_key));

  // previous version of the test relied on Block interface, which stored a set
  // of signatures by public key
  std::vector<decltype(sig)> sigs{1, sig};
  ASSERT_FALSE(hasSupermajority(sigs | boost::adaptors::indirected, peers));
}

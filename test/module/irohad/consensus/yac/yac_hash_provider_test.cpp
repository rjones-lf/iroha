/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "consensus/yac/impl/yac_hash_provider_impl.hpp"

#include <string>

#include <gtest/gtest.h>
#include <boost/make_shared.hpp>
#include <boost/range/adaptor/indirected.hpp>
#include <boost/shared_container_iterator.hpp>
#include <boost/shared_ptr.hpp>
#include "module/shared_model/interface_mocks.hpp"

using namespace iroha::consensus::yac;

using ::testing::Return;
using ::testing::ReturnRefOfCopy;

auto makeSignature() {
  auto signature = std::make_unique<MockSignature>();
  EXPECT_CALL(*signature, publicKey())
      .WillRepeatedly(ReturnRefOfCopy(shared_model::crypto::PublicKey("key")));
  EXPECT_CALL(*signature, signedData())
      .WillRepeatedly(ReturnRefOfCopy(shared_model::crypto::Signed("data")));
  return signature;
}

auto signature() {
  auto signature = makeSignature();
  EXPECT_CALL(*signature, clone())
      .WillRepeatedly(Return(makeSignature().release()));
  return signature;
}

TEST(YacHashProviderTest, MakeYacHashTest) {
  YacHashProviderImpl hash_provider;
  auto block = std::make_shared<MockBlock>();
  EXPECT_CALL(*block, payload())
      .WillRepeatedly(
          ReturnRefOfCopy(shared_model::crypto::Blob(std::string())));

  EXPECT_CALL(*block, signatures())
      .WillRepeatedly(
          Return(boost::make_shared_container_range(
                     boost::make_shared<std::vector<
                         std::shared_ptr<shared_model::interface::Signature>>>(
                         1, signature()))
                 | boost::adaptors::indirected));

  auto hex_test_hash = block->hash().hex();

  auto yac_hash = hash_provider.makeHash(*block);

  ASSERT_EQ(hex_test_hash, yac_hash.vote_hashes.proposal_hash);
  ASSERT_EQ(hex_test_hash, yac_hash.vote_hashes.block_hash);
}

TEST(YacHashProviderTest, ToModelHashTest) {
  YacHashProviderImpl hash_provider;
  auto block = std::make_shared<MockBlock>();
  EXPECT_CALL(*block, payload())
      .WillRepeatedly(
          ReturnRefOfCopy(shared_model::crypto::Blob(std::string())));

  EXPECT_CALL(*block, signatures())
      .WillRepeatedly(
          Return(boost::make_shared_container_range(
                     boost::make_shared<std::vector<
                         std::shared_ptr<shared_model::interface::Signature>>>(
                         1, signature()))
                 | boost::adaptors::indirected));

  auto yac_hash = hash_provider.makeHash(*block);

  auto model_hash = hash_provider.toModelHash(yac_hash);

  ASSERT_EQ(model_hash, block->hash());
}

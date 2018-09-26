/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "consensus/yac/impl/yac_hash_provider_impl.hpp"

#include <string>

#include <gtest/gtest.h>
#include "module/shared_model/builders/protobuf/common_objects/proto_signature_builder.hpp"
#include "module/shared_model/builders/protobuf/test_block_builder.hpp"

using namespace iroha::consensus::yac;

TEST(YacHashProviderTest, MakeYacHashTest) {
  YacHashProviderImpl hash_provider;
  auto block =
      std::make_shared<shared_model::proto::Block>(TestBlockBuilder().build());

  block->addSignature(shared_model::crypto::Signed("data"),
                      shared_model::crypto::PublicKey("key"));

  auto hex_test_hash = block->hash().hex();

  auto yac_hash = hash_provider.makeHash(*block);

  ASSERT_EQ(hex_test_hash, yac_hash.proposal_hash);
  ASSERT_EQ(hex_test_hash, yac_hash.block_hash);
}

TEST(YacHashProviderTest, ToModelHashTest) {
  YacHashProviderImpl hash_provider;
  auto block =
      std::make_shared<shared_model::proto::Block>(TestBlockBuilder().build());

  block->addSignature(shared_model::crypto::Signed("data"),
                      shared_model::crypto::PublicKey("key"));

  auto yac_hash = hash_provider.makeHash(*block);

  auto model_hash = hash_provider.toModelHash(yac_hash);

  ASSERT_EQ(model_hash, block->hash());
}

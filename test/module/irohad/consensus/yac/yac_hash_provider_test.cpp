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
#include "consensus/yac/impl/yac_hash_provider_impl.hpp"
#include <string>

using namespace iroha::consensus::yac;


TEST(YacHashProviderTest, MakeYacHashTest) {
  YacHashProviderImpl hash_provider;
  iroha::model::Block block;
  std::string test_hash = std::string(block.hash.size(), 'f');
  std::copy(test_hash.begin(), test_hash.end(), block.hash.begin());
  auto hex_test_hash = block.hash.to_hexstring();

  auto yac_hash = hash_provider.makeHash(block.hash);

  ASSERT_EQ(hex_test_hash, yac_hash.proposal_hash);
  ASSERT_EQ(hex_test_hash, yac_hash.block_hash);
}

TEST(YacHashProviderTest, ToModelHashTest) {
  YacHashProviderImpl hash_provider;
  iroha::model::Block block;
  block.hash.fill('f');

  auto yac_hash = hash_provider.makeHash(block.hash);

  auto model_hash = hash_provider.toModelHash(yac_hash);

  ASSERT_EQ(model_hash, block.hash);
}

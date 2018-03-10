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
#include "consensus/yac/impl/yac_crypto_provider_impl.hpp"
#include "consensus/yac/impl/yac_hash_provider_impl.hpp"
#include "consensus/yac/messages.hpp"
#include "cryptography/ed25519_sha3_impl/internal/ed25519_impl.hpp"

namespace iroha {
  namespace consensus {
    namespace yac {
      class YacCryptoProviderTest : public ::testing::Test {
       public:
        YacCryptoProviderTest() : keypair(create_keypair()) {}

        void SetUp() override {
          crypto_provider = std::make_shared<CryptoProviderImpl>(keypair);
        }

        const keypair_t keypair;
        std::shared_ptr<CryptoProviderImpl> crypto_provider;
      };

      TEST_F(YacCryptoProviderTest, ValidWhenSameMessage) {
        YacHash hash("1", "1");
        hash.block_signature.pubkey.fill('0');
        hash.block_signature.signature.fill('1');

        auto vote = crypto_provider->getVote(hash);

        ASSERT_TRUE(crypto_provider->verify(vote));
      }

      TEST_F(YacCryptoProviderTest, InvalidWhenMessageChanged) {
        YacHash hash("1", "1");
        hash.block_signature.pubkey.fill('0');
        hash.block_signature.signature.fill('1');

        auto vote = crypto_provider->getVote(hash);

        vote.hash.block_hash = "hash changed";

        ASSERT_FALSE(crypto_provider->verify(vote));
      }

    }  // namespace yac
  }    // namespace consensus
}  // namespace iroha

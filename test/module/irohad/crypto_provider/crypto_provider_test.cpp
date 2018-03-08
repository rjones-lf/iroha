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

#include "crypto_provider/impl/crypto_provider_impl.hpp"
#include "module/shared_model/builders/protobuf/test_block_builder.hpp"
#include "module/shared_model/builders/protobuf/test_query_builder.hpp"
#include "module/shared_model/builders/protobuf/test_transaction_builder.hpp"

#include "cryptography/crypto_provider/crypto_model_signer.hpp"

namespace iroha {
  class CryptoProviderTest : public ::testing::Test {
   public:
    virtual void SetUp() {
      auto creator = "a@domain";
      auto account_id = "b@domain";

      // initialize block
      block = std::make_unique<shared_model::proto::Block>(
          TestBlockBuilder().build());

      // initialize query
      query = std::make_unique<shared_model::proto::Query>(
          TestQueryBuilder()
              .creatorAccountId(creator)
              .queryCounter(1)
              .getAccount(account_id)
              .build());

      // initialize transaction
      transaction = std::make_unique<shared_model::proto::Transaction>(
          TestTransactionBuilder()
              .creatorAccountId(account_id)
              .txCounter(1)
              .setAccountQuorum(account_id, 2)
              .build());
    }

    template <typename T>
    void signIncorrect(T &signable) {
      // initialize wrong signature
      auto signedBlob = shared_model::crypto::CryptoSigner<>::sign(
          shared_model::crypto::Blob("wrong payload"), keypair);
      iroha::protocol::Signature wrong_signature;
      wrong_signature.set_pubkey(
          shared_model::crypto::toBinaryString(keypair.publicKey()));
      wrong_signature.set_signature(
          shared_model::crypto::toBinaryString(signedBlob));
      signable.addSignature(shared_model::detail::PolymorphicWrapper<
                            shared_model::proto::Signature>(
          new shared_model::proto::Signature(wrong_signature)));
    }

    shared_model::crypto::Keypair keypair =
        shared_model::crypto::DefaultCryptoAlgorithmType::generateKeypair();
    CryptoProviderImpl<> provider = CryptoProviderImpl<>(keypair);

    std::unique_ptr<shared_model::proto::Block> block;
    std::unique_ptr<shared_model::proto::Query> query;
    std::unique_ptr<shared_model::proto::Transaction> transaction;
  };

  /**
   * @given properly signed block
   * @when verify block
   * @then block is verified
   */
  TEST_F(CryptoProviderTest, SignAndVerifyBlock) {
    provider.sign(*block);

    ASSERT_TRUE(provider.verify(*block));
  }

  /**
   * @given block with inctorrect sign
   * @when verify block
   * @then block is not verified
   */
  TEST_F(CryptoProviderTest, SignAndVerifyBlockWithWrongSignature) {
    signIncorrect(*block);

    ASSERT_FALSE(provider.verify(*block));
  }

  /**
   * @given properly signed query
   * @when verify query
   * @then query is verified
   */
  TEST_F(CryptoProviderTest, SignAndVerifyQuery) {
    provider.sign(*query);

    ASSERT_TRUE(provider.verify(*query));
  }

  /**
   * @given query with incorrect sign
   * @when verify query
   * @then query is not verified
   */
  TEST_F(CryptoProviderTest, SignAndVerifyQuerykWithWrongSignature) {
    signIncorrect(*query);

    ASSERT_FALSE(provider.verify(*query));
  }

  /**
   * @given query hash
   * @when sign query
   * @then query hash doesn't change
   */
  TEST_F(CryptoProviderTest, SameQueryHashAfterSign) {
    auto hash_before = query->hash();
    provider.sign(*query);
    auto hash_signed = query->hash();

    ASSERT_EQ(hash_signed, hash_before);
  }

  /**
   * @given properly signed transaction
   * @when verify transaction
   * @then transaction is verified
   */
  TEST_F(CryptoProviderTest, SignAndVerifyTransaction) {
    provider.sign(*transaction);

    ASSERT_TRUE(provider.verify(*transaction));
  }

  /**
   * @given transaction with incorrect sign
   * @when verify transaction
   * @then transaction is not verified
   */
  TEST_F(CryptoProviderTest, SignAndVerifyTransactionkWithWrongSignature) {
    signIncorrect(*transaction);

    ASSERT_FALSE(provider.verify(*transaction));
  }

}  // namespace iroha

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
#include <crypto/crypto.hpp>
#include <model/model_crypto_provider_impl.hpp>
#include <model/model_hash_provider_impl.hpp>

using namespace iroha::model;

iroha::model::Signature create_signature() {
  iroha::model::Signature signature{};
  std::fill(signature.signature.begin(), signature.signature.end(), 0x0);
  std::fill(signature.pubkey.begin(), signature.pubkey.end(), 0x0);
  return signature;
}

iroha::model::Transaction create_transaction() {
  iroha::model::Transaction tx{};
  tx.creator_account_id = "test";

  tx.tx_counter = 0;
  tx.created_ts = 0;
  return tx;
}

iroha::model::Block create_block() {
  iroha::model::Block block{};
  std::fill(block.hash.begin(), block.hash.end(), 0x0);
  block.created_ts = 0;
  block.height = 0;
  std::fill(block.prev_hash.begin(), block.prev_hash.end(), 0x0);
  block.txs_number = 0;
  std::fill(block.merkle_root.begin(), block.merkle_root.end(), 0x0);
  block.transactions.push_back(create_transaction());
  return block;
}

Transaction sign(Transaction &tx, iroha::ed25519::privkey_t privkey,
                 iroha::ed25519::pubkey_t pubkey) {
  HashProviderImpl hash_provider;
  auto tx_hash = hash_provider.get_hash(tx);

  auto sign = iroha::sign(tx_hash.data(), tx_hash.size(), pubkey, privkey);

  Signature signature{};
  signature.signature = sign;
  signature.pubkey = pubkey;

  tx.signatures.push_back(signature);

  return tx;
}

TEST(CryptoProvider, SignAndVerify) {
  // generate privkey/pubkey keypair
  auto seed = iroha::create_seed();
  auto keypair = iroha::create_keypair(seed);

  auto model_tx = create_transaction();

  iroha::model::ModelCryptoProviderImpl crypto_provider(keypair.privkey,
                                                        keypair.pubkey);
  sign(model_tx, keypair.privkey, keypair.pubkey);
  ASSERT_TRUE(crypto_provider.verify(model_tx));

  // now modify transaction's meta, so verify should fail
  model_tx.creator_account_id = "test1";
  ASSERT_FALSE(crypto_provider.verify(model_tx));
}

TEST(CryptoProvider, SignAndVerifyBlock) {
  // generate privkey/pubkey keypair
  auto seed = iroha::create_seed();
  auto keypair = iroha::create_keypair(seed);

  auto block = create_block();

  iroha::model::ModelCryptoProviderImpl crypto_provider(keypair.privkey,
                                                        keypair.pubkey);
  crypto_provider.sign(block);
  ASSERT_TRUE(crypto_provider.verify(block));

  // now modify block's meta, so verify should fail
  block.txs_number += 1;
  ASSERT_FALSE(crypto_provider.verify(block));
}
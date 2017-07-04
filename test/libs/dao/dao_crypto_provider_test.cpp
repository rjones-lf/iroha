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
#include <dao/dao_crypto_provider_impl.hpp>
#include <crypto/crypto.hpp>

iroha::dao::Transaction create_transaction() {
  iroha::dao::Transaction tx{};
  memset(tx.creator.data(), 0x1, 32);

  tx.tx_counter = 0;
  tx.created_ts = 0;


//  iroha::dao::Signature signature1{};
//  signature1.pubkey = pubkey;
//  signature1.signature = sign;
//  tx.signatures.push_back(signature1);

  //  tx.commands
  return tx;
}

TEST(CryptoProvider, SignAndVerify){
  auto seed = iroha::create_seed();
  auto keypair = iroha::create_keypair(seed);

  auto dao_tx = create_transaction();

  iroha::dao::DaoCryptoProviderImpl crypto_provider(keypair.privkey, keypair.pubkey);
  crypto_provider.sign(dao_tx);
  ASSERT_TRUE(crypto_provider.verify(dao_tx));

  memset(dao_tx.creator.data(), 0x123, iroha::ed25519::pubkey_t::size());
  ASSERT_FALSE(crypto_provider.verify(dao_tx));
}
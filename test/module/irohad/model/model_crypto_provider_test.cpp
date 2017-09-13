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
#include <block.pb.h>
#include <crypto/hash.hpp>
#include <model/converters/pb_transaction_factory.hpp>

using namespace iroha::model;

iroha::model::Transaction create_transaction() {
  iroha::model::Transaction tx{};
  tx.creator_account_id = "test";

  tx.tx_counter = 0;
  tx.created_ts = 0;
  return tx;
}


TEST(CryptoProvider, SignAndVerify) {
  // generate privkey/pubkey keypair
  auto keypair = iroha::create_keypair();

  auto model_tx = create_transaction();
  auto proto_tx = iroha::model::converters::PbTransactionFactory::serialize(model_tx);

  iroha::hash256_t hash = iroha::sha3_256(proto_tx.payload().SerializeAsString());
  auto sig = iroha::sign(hash.to_string(), keypair.pubkey, keypair.privkey);
  ASSERT_TRUE(iroha::verify(hash.to_string(), keypair.pubkey, sig));

  // now modify transaction's creator, so verify should fail
  model_tx.creator_account_id = "test1";
  proto_tx = iroha::model::converters::PbTransactionFactory::serialize(model_tx);
  hash = iroha::sha3_256(proto_tx.payload().SerializeAsString());
  ASSERT_FALSE(iroha::verify(hash.to_string(), keypair.pubkey, sig));
}

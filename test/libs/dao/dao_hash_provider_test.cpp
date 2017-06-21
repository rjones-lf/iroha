/*
Copyright Soramitsu Co., Ltd. 2016 All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include <gtest/gtest.h>
#include <dao/dao_hash_provider_impl.hpp>

iroha::dao::Signature create_signature() {
  iroha::dao::Signature signature{};
  memset(signature.signature.data(), 0x0, 64);
  memset(signature.pubkey.data(), 0x0, 32);
  return signature;
}

iroha::dao::Transaction create_transaction() {
  iroha::dao::Transaction tx{};
  memset(tx.creator.data(), 0x1, 32);

  tx.tx_counter = 0;
  tx.created_ts = 0;

  tx.signatures.push_back(create_signature());
  tx.signatures.push_back(create_signature());

  //  tx.commands
  return tx;
}

TEST(DaoHashProviderTest, DaoHashProviderWhenHashTransactionIsCalled) {
  using iroha::dao::HashProviderImpl;
  using iroha::dao::HashProvider;

  std::unique_ptr<HashProvider<iroha::crypto::ed25519::PUBLEN>> hash_provider = std::make_unique<HashProviderImpl>();

  iroha::dao::Transaction tx = create_transaction();
  auto res = hash_provider->get_hash(tx);

  std::cout << "hash: " << iroha::crypto::digest_to_hexdigest(res.data(), 32) << std::endl;
}
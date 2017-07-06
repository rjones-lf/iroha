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
#include <common/types.hpp>
#include <dao/dao_hash_provider_impl.hpp>

iroha::dao::Signature create_signature();
iroha::dao::Transaction create_transaction();
iroha::dao::Proposal create_proposal();
iroha::dao::Block create_block();

iroha::dao::Signature create_signature() {
  iroha::dao::Signature signature{};
  memset(signature.signature.data(), 0x0, iroha::ed25519::sig_t::size());
  memset(signature.pubkey.data(), 0x0, iroha::ed25519::pubkey_t::size());
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

iroha::dao::Proposal create_proposal() {
  std::vector<iroha::dao::Transaction> txs;
  txs.push_back(create_transaction());
  txs.push_back(create_transaction());

  iroha::dao::Proposal proposal(txs);
  return proposal;
}

iroha::dao::Block create_block() {
  iroha::dao::Block block{};
  memset(block.hash.data(), 0x0, iroha::ed25519::pubkey_t::size());
  block.sigs.push_back(create_signature());
  block.created_ts = 0;
  block.height = 0;
  memset(block.prev_hash.data(), 0x0, iroha::ed25519::pubkey_t::size());
  block.txs_number = 0;
  memset(block.merkle_root.data(), 0x0, iroha::ed25519::pubkey_t::size());
  block.transactions.push_back(create_transaction());
  return block;
}

TEST(DaoHashProviderTest, DaoHashProviderWhenGetHashBlockIsCalled) {
  using iroha::dao::HashProviderImpl;
  using iroha::dao::HashProvider;

  std::unique_ptr<HashProvider<iroha::ed25519::pubkey_t::size()>>
      hash_provider = std::make_unique<HashProviderImpl>();

  auto block = create_block();
  auto res = hash_provider->get_hash(block);
  std::cout << "block hash: " << res.to_hexstring() << std::endl;
}

TEST(DaoHashProviderTest, DaoHashProviderWhenGetHashProposalIsCalled) {
  using iroha::dao::HashProviderImpl;
  using iroha::dao::HashProvider;

  std::unique_ptr<HashProvider<iroha::ed25519::pubkey_t::size()>>
      hash_provider = std::make_unique<HashProviderImpl>();

  iroha::dao::Proposal proposal = create_proposal();

  auto res = hash_provider->get_hash(proposal);
  std::cout << "proposal hash: " << res.to_hexstring() << std::endl;
}

TEST(DaoHashProviderTest, DaoHashProviderWhenGetHashTransactionIsCalled) {
  using iroha::dao::HashProviderImpl;
  using iroha::dao::HashProvider;

  std::unique_ptr<HashProvider<iroha::ed25519::pubkey_t::size()>>
      hash_provider = std::make_unique<HashProviderImpl>();

  iroha::dao::Transaction tx = create_transaction();
  auto res = hash_provider->get_hash(tx);

  std::cout << "transaction hash: " << res.to_hexstring() << std::endl;
}
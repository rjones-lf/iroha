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

#ifndef IROHA_BLOCK_QUERY_FIXTURE_HPP_
#define IROHA_BLOCK_QUERY_FIXTURE_HPP_

#include <boost/filesystem.hpp>
#include <boost/optional.hpp>
#include <memory>

#include "ametsuchi/block_query.hpp"
#include "ametsuchi/block_storage.hpp"
#include "ametsuchi/impl/block_index.hpp"

#include "framework/test_subscriber.hpp"
#include "model/converters/json_block_factory.hpp"
#include "model/sha3_hash.hpp"
#include "module/irohad/ametsuchi/ametsuchi_fixture.hpp"

using iroha::ametsuchi::AmetsuchiTest;
using iroha::ametsuchi::BlockStorage;
using iroha::model::Transaction;
using iroha::model::Block;
using iroha::ametsuchi::BlockIndex;
using iroha::ametsuchi::BlockQuery;
using iroha::model::converters::JsonBlockFactory;

class BlockQueryFixture : public AmetsuchiTest {
 public:
  BlockQueryFixture() {
    using namespace iroha::model;

    uint64_t height = AmetsuchiTest::FIRST_BLOCK;

    // First transaction in block1
    Transaction txn1_1;
    txn1_1.creator_account_id = creator1;
    tx_hashes.push_back(iroha::hash(txn1_1));

    // Second transaction in block1
    Transaction txn1_2;
    txn1_2.creator_account_id = creator1;
    tx_hashes.push_back(iroha::hash(txn1_2));

    Block block1;
    block1.height = height++;
    block1.transactions.push_back(txn1_1);
    block1.transactions.push_back(txn1_2);
    auto block1hash = iroha::hash(block1);

    // First tx in block 1
    Transaction txn2_1;
    txn2_1.creator_account_id = creator1;
    tx_hashes.push_back(iroha::hash(txn2_1));

    // Second tx in block 2
    Transaction txn2_2;
    // this tx has another creator
    txn2_2.creator_account_id = creator2;
    tx_hashes.push_back(iroha::hash(txn2_2));

    Block block2;
    block2.height = height++;
    block2.prev_hash = block1hash;
    block2.transactions.push_back(txn2_1);
    block2.transactions.push_back(txn2_2);

    blocks.push_back(std::move(block1));
    blocks.push_back(std::move(block2));
  }

  void SetUp() override {
    AmetsuchiTest::SetUp();
    conv = std::make_unique<JsonBlockFactory>();

    // TODO(@warchant): make separate postgres connector
    postgres_connection = std::make_unique<pqxx::lazyconnection>(pgopt_);
    try {
      postgres_connection->activate();
    } catch (const pqxx::broken_connection &e) {
      FAIL() << "Connection to PostgreSQL broken: " << e.what();
    }
    transaction = std::make_unique<pqxx::nontransaction>(
        *postgres_connection, "Postgres block indexes");
  }

  std::unique_ptr<pqxx::nontransaction> transaction;
  std::unique_ptr<pqxx::lazyconnection> postgres_connection;
  std::vector<Block> blocks;
  std::vector<iroha::hash256_t> tx_hashes;
  std::string creator1 = "user1@test";
  std::string creator2 = "user2@test";
  std::shared_ptr<BlockQuery> query;
  std::shared_ptr<BlockIndex> index;
  std::unique_ptr<BlockStorage> bs_;
  std::unique_ptr<JsonBlockFactory> conv;
};

#endif  //  IROHA_BLOCK_QUERY_FIXTURE_HPP_

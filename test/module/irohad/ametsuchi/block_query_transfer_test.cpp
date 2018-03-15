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

#include <boost/optional.hpp>
#include "ametsuchi/impl/postgres_block_index.hpp"
#include "ametsuchi/impl/postgres_block_query.hpp"
#include "backend/protobuf/from_old_model.hpp"
#include "framework/test_subscriber.hpp"
#include "module/irohad/ametsuchi/ametsuchi_fixture.hpp"
#include "module/shared_model/builders/protobuf/test_block_builder.hpp"
#include "module/shared_model/builders/protobuf/test_transaction_builder.hpp"

using namespace framework::test_subscriber;

namespace iroha {
  namespace ametsuchi {
    class BlockQueryTransferTest : public AmetsuchiTest {
     protected:
      void SetUp() override {
        AmetsuchiTest::SetUp();

        auto tmp = FlatFile::create(block_store_path);
        ASSERT_TRUE(tmp);
        file = std::move(*tmp);

        postgres_connection = std::make_unique<pqxx::lazyconnection>(pgopt_);
        try {
          postgres_connection->activate();
        } catch (const pqxx::broken_connection &e) {
          FAIL() << "Connection to PostgreSQL broken: " << e.what();
        }
        transaction = std::make_unique<pqxx::nontransaction>(
            *postgres_connection, "Postgres block indexes");

        index = std::make_shared<PostgresBlockIndex>(*transaction);
        blocks = std::make_shared<PostgresBlockQuery>(*transaction, *file);

        transaction->exec(init_);
      }

      void insert(const shared_model::interface::Block &block) {
        // TODO IR-975 victordrobny 12.02.2018 convert from
        // shared_model::proto::Block after FlatFile will be reworked to new
        // model
        auto old_block =
            *std::unique_ptr<iroha::model::Block>(block.makeOldModel());
        file->add(
            block.height(),
            iroha::stringToBytes(model::converters::jsonToString(
                model::converters::JsonBlockFactory().serialize(old_block))));
        index->index(block);
      }

      std::unique_ptr<pqxx::lazyconnection> postgres_connection;
      std::unique_ptr<pqxx::nontransaction> transaction;
      std::vector<shared_model::crypto::Hash> tx_hashes;
      std::shared_ptr<BlockQuery> blocks;
      std::shared_ptr<BlockIndex> index;
      std::unique_ptr<FlatFile> file;
      std::string creator1 = "user1@test";
      std::string creator2 = "user2@test";
      std::string creator3 = "user3@test";
      std::string asset = "coin#test";
    };

    auto zero_string = std::string("0", 32);
    auto fake_hash = shared_model::crypto::Hash(zero_string);

    /**
     * Make block with one transaction(transfer 0 asset) with specified sender,
     * receiver, asset and creator of transaction
     * @param creator1 - source account for transfer
     * @param creator2 - dest account for transfer
     * @param asset - asset for transfer
     * @param tx_creator - creator of the transaction
     * @return block with one transaction
     */
    shared_model::proto::Block makeBlockWithCreator(std::string creator1,
                                                    std::string creator2,
                                                    std::string asset,
                                                    std::string tx_creator) {
      return TestBlockBuilder()
          .transactions(std::vector<shared_model::proto::Transaction>(
              {TestTransactionBuilder()
                   .creatorAccountId(tx_creator)
                   .transferAsset(
                       creator1, creator2, asset, "Transfer asset", "0.0")
                   .build()}))
          .height(1)
          .prevHash(fake_hash)
          .build();
    }

    /**
     * Make block with one transaction(transfer 0 asset) with specified sender,
     * receiver and asset
     * @param creator1 - source account for transfer
     * @param creator2 - dest account for transfer
     * @param asset - asset for transfer
     * @return block with one transaction
     */
    shared_model::proto::Block makeBlock(
        std::string creator1,
        std::string creator2,
        std::string asset,
        int height = 1,
        shared_model::crypto::Hash hash = fake_hash) {
      return TestBlockBuilder()
          .transactions(std::vector<shared_model::proto::Transaction>(
              {TestTransactionBuilder()
                   .transferAsset(
                       creator1, creator2, asset, "Transfer asset", "0.0")
                   .build()}))
          .height(height)
          .prevHash(hash)
          .build();
    }

    /**
     * @given block store and index with block containing 1 transaction
     * with transfer from creator 1 to creator 2 sending asset
     * @when query to get asset transactions of sender
     * @then query returns the transaction
     */
    TEST_F(BlockQueryTransferTest, SenderAssetName) {
      auto block = makeBlockWithCreator(creator1, creator2, asset, creator1);
      tx_hashes.push_back(block.transactions().back()->hash());
      insert(block);

      auto wrapper = make_test_subscriber<CallExact>(
          blocks->getAccountAssetTransactions(creator1, asset), 1);
      wrapper.subscribe(
          [this](auto val) { ASSERT_EQ(tx_hashes.at(0), val->hash()); });
      ASSERT_TRUE(wrapper.validate());
    }

    /**
     * @given block store and index with block containing 1 transaction
     * with transfer from creator 1 to creator 2 sending asset
     * @when query to get asset transactions of receiver
     * @then query returns the transaction
     */
    TEST_F(BlockQueryTransferTest, ReceiverAssetName) {
      auto block = makeBlockWithCreator(creator1, creator2, asset, creator1);
      tx_hashes.push_back(block.transactions().back()->hash());
      insert(block);

      auto wrapper = make_test_subscriber<CallExact>(
          blocks->getAccountAssetTransactions(creator2, asset), 1);
      wrapper.subscribe(
          [this](auto val) { ASSERT_EQ(tx_hashes.at(0), val->hash()); });
      ASSERT_TRUE(wrapper.validate());
    }

    /**
     * @given block store and index with block containing 1 transaction
     * from creator 3 with transfer from creator 1 to creator 2 sending asset
     * @when query to get asset transactions of transaction creator
     * @then query returns the transaction
     */
    TEST_F(BlockQueryTransferTest, GrantedTransfer) {
      auto block = makeBlockWithCreator(creator1, creator2, asset, creator3);
      tx_hashes.push_back(block.transactions().back()->hash());
      insert(block);

      auto wrapper = make_test_subscriber<CallExact>(
          blocks->getAccountAssetTransactions(creator3, asset), 1);
      wrapper.subscribe(
          [this](auto val) { ASSERT_EQ(tx_hashes.at(0), val->hash()); });
      ASSERT_TRUE(wrapper.validate());
    }

    /**
     * @given block store and index with 2 blocks containing 1 transaction
     * with transfer from creator 1 to creator 2 sending asset
     * @when query to get asset transactions of sender
     * @then query returns the transactions
     */
    TEST_F(BlockQueryTransferTest, TwoBlocks) {
      auto block = makeBlock(creator1, creator2, asset);

      tx_hashes.push_back(block.transactions().back()->hash());
      insert(block);

      auto block2 = makeBlock(creator1, creator2, asset, 2, block.hash());

      tx_hashes.push_back(block.transactions().back()->hash());
      insert(block2);

      auto wrapper = make_test_subscriber<CallExact>(
          blocks->getAccountAssetTransactions(creator1, asset), 2);
      wrapper.subscribe([i = 0, this](auto val) mutable {
        ASSERT_EQ(tx_hashes.at(i), val->hash());
        ++i;
      });
      ASSERT_TRUE(wrapper.validate());
    }
  }  // namespace ametsuchi
}  // namespace iroha

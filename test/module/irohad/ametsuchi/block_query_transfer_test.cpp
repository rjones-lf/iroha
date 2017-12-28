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
#include "ametsuchi/impl/redis_block_index.hpp"
#include "ametsuchi/impl/redis_block_query.hpp"
#include "cryptography/ed25519_sha3_impl/internal/sha3_hash.hpp"
#include "framework/test_subscriber.hpp"
#include "model/commands/transfer_asset.hpp"
#include "module/irohad/ametsuchi/ametsuchi_fixture.hpp"

using namespace framework::test_subscriber;

namespace iroha {
  namespace ametsuchi {
    class BlockQueryTransferTest : public AmetsuchiTest {
     protected:
      void SetUp() override {
        AmetsuchiTest::SetUp();

        file = FlatFile::create(block_store_path);
        ASSERT_TRUE(file);
        index = std::make_shared<RedisBlockIndex>(client);
        blocks = std::make_shared<RedisBlockQuery>(client, *file);
      }

      void insert(const model::Block &block) {
        file->add(block.height,
                  iroha::stringToBytes(model::converters::jsonToString(
                      model::converters::JsonBlockFactory().serialize(block))));
        index->index(block);
      }

      std::vector<iroha::hash256_t> tx_hashes;
      std::shared_ptr<BlockQuery> blocks;
      std::shared_ptr<BlockIndex> index;
      std::unique_ptr<FlatFile> file;
      std::string creator1 = "user1@test";
      std::string creator2 = "user2@test";
      std::string creator3 = "user3@test";
      std::string asset = "coin#test";
      std::vector<std::string> assets = {asset};
    };

    /**
     * @given block store and index with block containing 1 transaction
     * with transfer from creator 1 to creator 2 sending asset
     * @when query to get asset transactions of sender
     * @then query returns the transaction
     */
    TEST_F(BlockQueryTransferTest, SenderAssetName) {
      model::Block block;
      block.transactions.emplace_back();

      block.transactions.back().commands.push_back(
          cmd_gen.generateTransferAsset(creator1, creator2, asset, {}));
      tx_hashes.push_back(iroha::hash(block.transactions.back()));

      block.height = 1;
      insert(block);

      auto wrapper = make_test_subscriber<CallExact>(
          blocks->getAccountAssetTransactions(creator1, assets), 1);
      wrapper.subscribe(
          [this](auto val) { ASSERT_EQ(tx_hashes.at(0), iroha::hash(val)); });
      ASSERT_TRUE(wrapper.validate());
    }

    /**
     * @given block store and index with block containing 1 transaction
     * with transfer from creator 1 to creator 2 sending asset
     * @when query to get asset transactions of receiver
     * @then query returns the transaction
     */
    TEST_F(BlockQueryTransferTest, ReceiverAssetName) {
      model::Block block;
      block.transactions.emplace_back();

      block.transactions.back().commands.push_back(
          cmd_gen.generateTransferAsset(creator1, creator2, asset, {}));
      tx_hashes.push_back(iroha::hash(block.transactions.back()));

      block.height = 1;
      insert(block);

      auto wrapper = make_test_subscriber<CallExact>(
          blocks->getAccountAssetTransactions(creator2, assets), 1);
      wrapper.subscribe(
          [this](auto val) { ASSERT_EQ(tx_hashes.at(0), iroha::hash(val)); });
      ASSERT_TRUE(wrapper.validate());
    }

    /**
     * @given block store and index with block containing 1 transaction
     * from creator 3 with transfer from creator 1 to creator 2 sending asset
     * @when query to get asset transactions of transaction creator
     * @then query returns the transaction
     */
    TEST_F(BlockQueryTransferTest, GrantedTransfer) {
      model::Block block;
      block.transactions.emplace_back();
      block.transactions.back().creator_account_id = creator3;

      block.transactions.back().commands.push_back(
          cmd_gen.generateTransferAsset(creator1, creator2, asset, {}));
      tx_hashes.push_back(iroha::hash(block.transactions.back()));

      block.height = 1;
      insert(block);

      auto wrapper = make_test_subscriber<CallExact>(
          blocks->getAccountAssetTransactions(creator3, assets), 1);
      wrapper.subscribe(
          [this](auto val) { ASSERT_EQ(tx_hashes.at(0), iroha::hash(val)); });
      ASSERT_TRUE(wrapper.validate());
    }

    /**
     * @given block store and index with 2 blocks containing 1 transaction
     * with transfer from creator 1 to creator 2 sending asset
     * @when query to get asset transactions of sender
     * @then query returns the transactions
     */
    TEST_F(BlockQueryTransferTest, TwoBlocks) {
      model::Block block;
      block.transactions.emplace_back();

      block.transactions.back().commands.push_back(
          cmd_gen.generateTransferAsset(creator1, creator2, asset, {}));
      tx_hashes.push_back(iroha::hash(block.transactions.back()));

      block.height = 1;
      insert(block);

      block = model::Block();
      block.transactions.emplace_back();

      block.transactions.back().commands.push_back(
          cmd_gen.generateTransferAsset(creator1, creator2, asset, {}));
      tx_hashes.push_back(iroha::hash(block.transactions.back()));

      block.height = 2;
      insert(block);

      auto wrapper = make_test_subscriber<CallExact>(
          blocks->getAccountAssetTransactions(creator1, assets), 2);
      wrapper.subscribe([ i = 0, this ](auto val) mutable {
        ASSERT_EQ(tx_hashes.at(i), iroha::hash(val));
        ++i;
      });
      ASSERT_TRUE(wrapper.validate());
    }
  }  // namespace ametsuchi
}  // namespace iroha

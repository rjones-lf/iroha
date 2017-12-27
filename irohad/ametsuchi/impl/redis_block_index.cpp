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

#include "ametsuchi/impl/redis_block_index.hpp"

#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/adaptor/indexed.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/algorithm/for_each.hpp>
#include <boost/range/numeric.hpp>
#include <unordered_map>
#include <unordered_set>

#include "cryptography/ed25519_sha3_impl/internal/sha3_hash.hpp"
#include "model/commands/transfer_asset.hpp"

namespace iroha {
  namespace ametsuchi {

    RedisBlockIndex::RedisBlockIndex(cpp_redis::client &client)
        : client_(client),
          account_id_height_("%s:%s"),
          account_id_height_asset_id_("%s:%s:%s") {}

    void RedisBlockIndex::index(const model::Block &block) {
      const auto &height = std::to_string(block.height);
      boost::for_each(
          block.transactions | boost::adaptors::indexed(0),
          [&](const auto &tx) {
            const auto &creator_id = tx.value().creator_account_id;
            const auto &hash = iroha::hash(tx.value()).to_string();
            const auto &index = std::to_string(tx.index());

            // tx hash -> block where hash is stored
            client_.set(hash, height);

            this->indexAccountIdHeight(creator_id, height);

            // to make index account_id:height -> list of tx indexes (where
            // tx is placed in the block)
            client_.rpush(boost::str(account_id_height_ % creator_id % height),
                          {index});

            this->indexAccountAssets(
                creator_id, height, index, tx.value().commands);
          });
    }

    void RedisBlockIndex::indexAccountIdHeight(const std::string &account_id,
                                               const std::string &height) {
      client_.sadd(account_id, {height});
    }

    void RedisBlockIndex::indexAccountAssets(
        const std::string &account_id,
        const std::string &height,
        const std::string &index,
        const model::Transaction::CommandsType &commands) {
      using UserAssetsType =
          std::unordered_map<std::string, std::unordered_set<std::string>>;

      // flat map abstract commands to transfers
      auto transfers =
          commands | boost::adaptors::transformed([](const auto &cmd) {
            return std::dynamic_pointer_cast<model::TransferAsset>(cmd);
          })
          | boost::adaptors::filtered(
                [](const auto &cmd) { return bool(cmd); });

      boost::accumulate(
          transfers,
          UserAssetsType{},
          [&](auto &&acc, const auto &cmd) {
            for (const auto &id : {cmd->src_account_id, cmd->dest_account_id}) {
              this->indexAccountIdHeight(id, height);
            }

            auto ids = {account_id, cmd->src_account_id, cmd->dest_account_id};
            // flat map accounts to unindexed keys
            auto unindexed =
                ids | boost::adaptors::transformed([&](const auto &id) {
                  return boost::str(account_id_height_asset_id_ % id % height
                                    % cmd->asset_id);
                })
                | boost::adaptors::filtered([&](const auto &key) {
                    return acc[key].insert(index).second;
                  });
            boost::for_each(unindexed, [&](const auto &key) {
              client_.rpush(key, {index});
            });

            return std::forward<decltype(acc)>(acc);
          });
    }
  }  // namespace ametsuchi
}  // namespace iroha

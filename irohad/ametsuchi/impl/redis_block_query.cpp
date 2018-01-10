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

#include "ametsuchi/impl/redis_block_query.hpp"
#include "cryptography/ed25519_sha3_impl/internal/sha3_hash.hpp"

namespace iroha {
  namespace ametsuchi {

    RedisBlockQuery::RedisBlockQuery(cpp_redis::client &client,
                                     FlatFile &file_store)
        : block_store_(file_store), client_(client) {}

    rxcpp::observable<model::Block> RedisBlockQuery::getBlocks(uint32_t height,
                                                               uint32_t count) {
      auto to = height + count;
      auto last_id = block_store_.last_id();
      to = std::min(to, last_id);
      if (height > to) {
        return rxcpp::observable<>::empty<model::Block>();
      }
      return rxcpp::observable<>::range(height, to).flat_map([this](auto i) {
        auto bytes = block_store_.get(i);
        return rxcpp::observable<>::create<model::Block>([this, bytes](auto s) {
          if (not bytes.has_value()) {
            s.on_completed();
            return;
          }
          auto document =
              model::converters::stringToJson(bytesToString(bytes.value()));
          if (not document.has_value()) {
            s.on_completed();
            return;
          }
          auto block = serializer_.deserialize(document.value());
          if (not block.has_value()) {
            s.on_completed();
            return;
          }
          s.on_next(block.value());
          s.on_completed();
        });
      });
    }

    rxcpp::observable<model::Block> RedisBlockQuery::getBlocksFrom(
        uint32_t height) {
      return getBlocks(height, block_store_.last_id());
    }

    rxcpp::observable<model::Block> RedisBlockQuery::getTopBlocks(
        uint32_t count) {
      auto last_id = block_store_.last_id();
      count = std::min(count, last_id);
      return getBlocks(last_id - count + 1, count);
    }

    std::vector<iroha::model::Block::BlockHeightType>
    RedisBlockQuery::getBlockIds(const std::string &account_id) {
      std::vector<uint64_t> block_ids;
      client_.smembers(account_id, [&block_ids](cpp_redis::reply &reply) {
        for (const auto &block_reply : reply.as_array()) {
          const auto &string_reply = block_reply.as_string();

          // check if reply is an integer
          if (isdigit(string_reply.c_str()[0])) {
            block_ids.push_back(std::stoul(string_reply));
          }
        }
      });
      client_.sync_commit();
      return block_ids;
    }

    boost::optional<iroha::model::Block::BlockHeightType>
    RedisBlockQuery::getBlockId(const std::string &hash) {
      boost::optional<uint64_t> blockId;
      client_.get(hash, [&blockId](cpp_redis::reply &reply) {
        if (reply.is_null()) {
          blockId = boost::none;
        } else {
          blockId = std::stoul(reply.as_string());
        }
      });
      client_.sync_commit();

      return blockId;
    }

    std::function<void(cpp_redis::reply &)> RedisBlockQuery::callbackToLrange(
        const rxcpp::subscriber<model::Transaction> &s, uint64_t block_id) {
      return [this, &s, block_id](cpp_redis::reply &reply) {
        auto tx_ids_reply = reply.as_array();

        block_store_.get(block_id) | [](auto bytes) {
          return model::converters::stringToJson(bytesToString(bytes));
        } | [this](const auto &json) {
          return serializer_.deserialize(json);
        } | [&](const auto &block) {
          for (const auto &tx_reply : tx_ids_reply) {
            auto tx_id = std::stoul(tx_reply.as_string());
            auto &&tx = block.transactions.at(tx_id);
            s.on_next(tx);
          }
        };
      };
    }

    rxcpp::observable<model::Transaction>
    RedisBlockQuery::getAccountTransactions(const std::string &account_id) {
      return rxcpp::observable<>::create<model::Transaction>(
          [this, account_id](auto subscriber) {
            auto block_ids = this->getBlockIds(account_id);
            if (block_ids.empty()) {
              subscriber.on_completed();
              return;
            }

            for (auto block_id : block_ids) {
              client_.lrange(account_id + ":" + std::to_string(block_id),
                             0,
                             -1,
                             this->callbackToLrange(subscriber, block_id));
            }
            client_.sync_commit();
            subscriber.on_completed();
          });
    }

    rxcpp::observable<model::Transaction>
    RedisBlockQuery::getAccountAssetTransactions(const std::string &account_id,
                                                 const std::string &asset_id) {
      return rxcpp::observable<>::create<model::Transaction>(
          [this, account_id, asset_id](auto subscriber) {
            auto block_ids = this->getBlockIds(account_id);
            if (block_ids.empty()) {
              subscriber.on_completed();
              return;
            }

            for (auto block_id : block_ids) {
              // create key for querying redis
              std::stringstream account_assets_key;
              account_assets_key << account_id << ':' << block_id << ':'
                                 << asset_id;
              client_.lrange(account_assets_key.str(),
                             0,
                             -1,
                             this->callbackToLrange(subscriber, block_id));
            }
            client_.sync_commit();
            subscriber.on_completed();
          });
    }

    rxcpp::observable<boost::optional<model::Transaction>>
    RedisBlockQuery::getTransactions(
        const std::vector<iroha::hash256_t> &tx_hashes) {
      return rxcpp::observable<>::create<boost::optional<model::Transaction>>(
          [this, tx_hashes](auto subscriber) {
            std::for_each(tx_hashes.begin(),
                          tx_hashes.end(),
                          [ that = this, &subscriber ](auto tx_hash) {
                            subscriber.on_next(
                                that->getTxByHashSync(tx_hash.to_string()));
                          });
            subscriber.on_completed();
          });
    }

    boost::optional<model::Transaction> RedisBlockQuery::getTxByHashSync(
        const std::string &hash) {
      return getBlockId(hash) |
          [this](auto blockId) { return block_store_.get(blockId); } |
          [](auto bytes) {
            return model::converters::stringToJson(bytesToString(bytes));
          }
      | [this](const auto &json) { return serializer_.deserialize(json); }
      | [&](const auto &block) {
          auto it = std::find_if(
              block.transactions.begin(),
              block.transactions.end(),
              [&hash](auto tx) { return iroha::hash(tx).to_string() == hash; });
          return (it == block.transactions.end())
              ? boost::none
              : boost::optional<model::Transaction>(*it);
        };
    }

  }  // namespace ametsuchi
}  // namespace iroha

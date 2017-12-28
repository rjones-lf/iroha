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
#include "model/commands/transfer_asset.hpp"
#include "model/commands/add_asset_quantity.hpp"

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

    rxcpp::observable<model::Transaction> RedisBlockQuery::reverseObservable(
        const rxcpp::observable<model::Transaction> &o) const {
      std::deque<model::Transaction> reverser;
      o.subscribe([&reverser](auto tx) { reverser.push_front(tx); });
      return rxcpp::observable<>::iterate(reverser);
    }

    rxcpp::observable<model::Transaction>
    RedisBlockQuery::getAccountTransactions(const std::string &account_id,
                                            const model::Pager &pager) {
      // TODO 06/11/17 motxx: Use Redis for getting hash and transactions
      return reverseObservable(
          getBlocksFrom(1)
              .flat_map([](auto block) {
                return rxcpp::observable<>::iterate(block.transactions);
              })
              .take_while([&pager](auto tx) {
                return iroha::hash(tx) != pager.tx_hash;
              })
              // filter txs by specified creator after take_while until tx_hash
              // to deal with other creator's tx_hash
              .filter([&account_id](auto tx) {
                return tx.creator_account_id == account_id;
              })
              // size of retrievable blocks and transactions should be
              // restricted in stateless validation.
              .take_last(pager.limit));
    }

    bool RedisBlockQuery::hasAccountAssetRelatedCommand(
        const std::string &account_id,
        const std::vector<std::string> &assets_id,
        const std::shared_ptr<iroha::model::Command> &command) const {
      auto predicate_producer = [&](const auto &check_target_id) {
        return [&check_target_id, &assets_id](const auto &com) {
          return check_target_id(com)
                 and std::any_of(assets_id.begin(),
                                 assets_id.end(),
                                 [&com](const auto &asset_id) {
                                   return asset_id == com.asset_id;
                                 });
        };
      };

      return isCommandValid<model::TransferAsset>(
        command,
        predicate_producer([&account_id](const auto &transfer) {
          return transfer.src_account_id == account_id
                 or transfer.dest_account_id == account_id;
        }))
             or isCommandValid<model::AddAssetQuantity>(
        command, predicate_producer([&account_id](const auto &add) {
          return add.account_id == account_id;
        }));
    }

    rxcpp::observable<model::Transaction>
    RedisBlockQuery::getAccountAssetTransactions(
        const std::string &account_id,
        const std::vector<std::string> &assets_id,
        const model::Pager &pager) {
      // TODO 06/11/17 motxx: Improve API by Redis
      return reverseObservable(
          getBlocksFrom(1)
              .flat_map([](auto block) {
                return rxcpp::observable<>::iterate(block.transactions);
              })
              // local variables can be captured because this observable will be
              // subscribed in this function.
              .take_while([&pager](auto tx) {
                return iroha::hash(tx) != pager.tx_hash;
              })
              .filter([this, &account_id, &assets_id](auto tx) {
                return std::any_of(
                    tx.commands.begin(),
                    tx.commands.end(),
                    [this, &account_id, &assets_id](auto command) {
                      // This "this->" is required by gcc.
                      return this->hasAccountAssetRelatedCommand(
                          account_id, assets_id, command);
                    });
              })
              // size of retrievable blocks and transactions should be
              // restricted in stateless validation.
              .take_last(pager.limit));
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

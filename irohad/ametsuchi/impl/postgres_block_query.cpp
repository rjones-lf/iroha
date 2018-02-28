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

#include "ametsuchi/impl/postgres_block_query.hpp"
#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/algorithm/for_each.hpp>
#include "model/sha3_hash.hpp"

namespace iroha {
  namespace ametsuchi {

    PostgresBlockQuery::PostgresBlockQuery(pqxx::nontransaction &transaction,
                                           FlatFile &file_store)
        : block_store_(file_store),
          transaction_(transaction),
          log_(logger::log("PostgresBlockIndex")),
          execute_{makeExecuteOptional(transaction_, log_)} {}

    rxcpp::observable<model::Block> PostgresBlockQuery::getBlocks(
        uint32_t height, uint32_t count) {
      auto last_id = block_store_.last_id();
      auto to = std::min(last_id, height + count - 1);
      if (height > to or count == 0) {
        return rxcpp::observable<>::empty<model::Block>();
      }

      return rxcpp::observable<>::range(height, to).flat_map([this](auto i) {
        auto bytes = block_store_.get(i);
        return rxcpp::observable<>::create<model::Block>([this, bytes](auto s) {
          if (not bytes) {
            s.on_completed();
            return;
          }
          auto document =
              model::converters::stringToJson(bytesToString(bytes.value()));
          if (not document) {
            s.on_completed();
            return;
          }
          auto block = serializer_.deserialize(document.value());
          if (not block) {
            s.on_completed();
            return;
          }
          s.on_next(block.value());
          s.on_completed();
        });
      });
    }

    rxcpp::observable<model::Block> PostgresBlockQuery::getBlocksFrom(
        uint32_t height) {
      return getBlocks(height, block_store_.last_id());
    }

    rxcpp::observable<model::Block> PostgresBlockQuery::getTopBlocks(
        uint32_t count) {
      auto last_id = block_store_.last_id();
      count = std::min(count, last_id);
      return getBlocks(last_id - count + 1, count);
    }

    std::vector<iroha::model::Block::BlockHeightType>
    PostgresBlockQuery::getBlockIds(const std::string &account_id) {
      return execute_(
                 "SELECT DISTINCT height FROM height_by_account_set WHERE "
                 "account_id = "
                 + transaction_.quote(account_id) + ";")
                 | [&](const auto &result)
                 -> std::vector<iroha::model::Block::BlockHeightType> {
        return transform<iroha::model::Block::BlockHeightType>(
            result, [&](const auto &row) {
              return row.at("height")
                  .template as<iroha::model::Block::BlockHeightType>();
            });
      };
    }

    boost::optional<iroha::model::Block::BlockHeightType>
    PostgresBlockQuery::getBlockId(const std::string &hash) {
      boost::optional<uint64_t> blockId;
      return execute_("SELECT height FROM height_by_hash WHERE hash = "
                      + transaction_.quote(
                            pqxx::binarystring(hash.data(), hash.size()))
                      + ";")
                 | [&](const auto &result)
                 -> boost::optional<iroha::model::Block::BlockHeightType> {
        if (result.size() == 0) {
          return boost::none;
        }
        return result[0]
            .at("height")
            .template as<iroha::model::Block::BlockHeightType>();
      };
    }

    std::function<void(pqxx::result &result)> PostgresBlockQuery::callback(
        const rxcpp::subscriber<model::Transaction> &subscriber,
        uint64_t block_id) {
      return [this, &subscriber, block_id](pqxx::result &result) {
        auto block = block_store_.get(block_id) | [](auto bytes) {
          return model::converters::stringToJson(bytesToString(bytes));
        } | [this](const auto &json) { return serializer_.deserialize(json); };
        boost::for_each(
            result | boost::adaptors::transformed([&block](const auto &x) {
              return x.at("index").template as<size_t>();
            }),
            [&](auto x) {
              const auto &tx = block->transactions.at(x);
              subscriber.on_next(tx);
            });
      };
    }

    rxcpp::observable<model::Transaction>
    PostgresBlockQuery::getAccountTransactions(const std::string &account_id) {
      return rxcpp::observable<>::create<model::Transaction>(
          [this, account_id](auto subscriber) {
            auto block_ids = this->getBlockIds(account_id);
            if (block_ids.empty()) {
              subscriber.on_completed();
              return;
            }

            for (const auto &block_id : block_ids) {
              execute_(
                  "SELECT DISTINCT index FROM index_by_creator_height WHERE "
                  "creator_id = "
                  + transaction_.quote(account_id)
                  + " AND height = " + transaction_.quote(block_id) + ";")
                  | this->callback(subscriber, block_id);
            }
            subscriber.on_completed();
          });
    }

    rxcpp::observable<model::Transaction>
    PostgresBlockQuery::getAccountAssetTransactions(
        const std::string &account_id, const std::string &asset_id) {
      return rxcpp::observable<>::create<
          model::Transaction>([this, account_id, asset_id](auto subscriber) {
        auto block_ids = this->getBlockIds(account_id);
        if (block_ids.empty()) {
          subscriber.on_completed();
          return;
        }

        for (const auto &block_id : block_ids) {
          // create key for querying redis
          execute_(
              "SELECT DISTINCT index FROM index_by_id_height_asset WHERE id = "
              + transaction_.quote(account_id)
              + " AND height = " + transaction_.quote(block_id)
              + " AND asset_id = " + transaction_.quote(asset_id) + ";")
              | this->callback(subscriber, block_id);
        }
        subscriber.on_completed();
      });
    }

    rxcpp::observable<boost::optional<model::Transaction>>
    PostgresBlockQuery::getTransactions(
        const std::vector<iroha::hash256_t> &tx_hashes) {
      return rxcpp::observable<>::create<boost::optional<model::Transaction>>(
          [this, tx_hashes](auto subscriber) {
            std::for_each(tx_hashes.begin(),
                          tx_hashes.end(),
                          [that = this, &subscriber](auto tx_hash) {
                            subscriber.on_next(
                                that->getTxByHashSync(tx_hash.to_string()));
                          });
            subscriber.on_completed();
          });
    }

    boost::optional<model::Transaction> PostgresBlockQuery::getTxByHashSync(
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

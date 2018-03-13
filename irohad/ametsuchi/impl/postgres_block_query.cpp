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
#include "backend/protobuf/from_old_model.hpp"

namespace iroha {
  namespace ametsuchi {

    PostgresBlockQuery::PostgresBlockQuery(pqxx::nontransaction &transaction,
                                           FlatFile &file_store)
        : block_store_(file_store),
          transaction_(transaction),
          log_(logger::log("PostgresBlockIndex")),
          execute_{makeExecuteOptional(transaction_, log_)} {}

    rxcpp::observable<BlockQuery::wBlock> PostgresBlockQuery::getBlocks(
        shared_model::interface::types::HeightType height, uint32_t count) {
      shared_model::interface::types::HeightType last_id =
          block_store_.last_id();
      auto to = std::min(last_id, height + count - 1);
      if (height > to or count == 0) {
        return rxcpp::observable<>::empty<wBlock>();
      }
      return rxcpp::observable<>::range(height, to).flat_map([this](auto i) {
        // TODO IR-975 victordrobny 12.02.2018 convert directly to
        // shared_model::proto::Block after FlatFile will be reworked to new
        // model
        auto block = block_store_.get(i) | [](const auto &bytes) {
          return model::converters::stringToJson(bytesToString(bytes));
        } | [this](const auto &d) {
          return serializer_.deserialize(d);
        } | [](const auto &block_old) {
          return std::make_shared<shared_model::proto::Block>(
              shared_model::proto::from_old(block_old));
        };
        return rxcpp::observable<>::create<PostgresBlockQuery::wBlock>(
            [this, block{std::move(block)}](auto s) {
              if (block) {
                s.on_next(block);
              }
              s.on_completed();
            });
      });
    }

    rxcpp::observable<BlockQuery::wBlock> PostgresBlockQuery::getBlocksFrom(
        shared_model::interface::types::HeightType height) {
      return getBlocks(height, block_store_.last_id());
    }

    rxcpp::observable<BlockQuery::wBlock> PostgresBlockQuery::getTopBlocks(
        uint32_t count) {
      auto last_id = block_store_.last_id();
      count = std::min(count, last_id);
      return getBlocks(last_id - count + 1, count);
    }

    std::vector<shared_model::interface::types::HeightType>
    PostgresBlockQuery::getBlockIds(
        const shared_model::interface::types::AccountIdType &account_id) {
      return execute_(
                 "SELECT DISTINCT height FROM height_by_account_set WHERE "
                 "account_id = "
                 + transaction_.quote(account_id) + ";")
                 | [&](const auto &result)
                 -> std::vector<shared_model::interface::types::HeightType> {
        return transform<shared_model::interface::types::HeightType>(
            result, [&](const auto &row) {
              return row.at("height")
                  .template as<shared_model::interface::types::HeightType>();
            });
      };
    }

    boost::optional<shared_model::interface::types::HeightType>
    PostgresBlockQuery::getBlockId(const shared_model::crypto::Hash &hash) {
      boost::optional<uint64_t> blockId;
      return execute_("SELECT height FROM height_by_hash WHERE hash = "
                      + transaction_.quote(pqxx::binarystring(
                            hash.blob().data(), hash.blob().size()))
                      + ";")
                 | [&](const auto &result)
                 -> boost::optional<
                     shared_model::interface::types::HeightType> {
        if (result.size() == 0) {
          return boost::none;
        }
        return result[0]
            .at("height")
            .template as<shared_model::interface::types::HeightType>();
      };
    }

    std::function<void(pqxx::result &result)> PostgresBlockQuery::callback(
        const rxcpp::subscriber<wTransaction> &subscriber, uint64_t block_id) {
      return [this, &subscriber, block_id](pqxx::result &result) {
        auto block = block_store_.get(block_id) | [this](auto bytes) {
          // TODO IR-975 victordrobny 12.02.2018 convert directly to
          // shared_model::proto::Block after FlatFile will be reworked to new
          // model
          return boost::optional<shared_model::proto::Block>(
              shared_model::proto::from_old(*serializer_.deserialize(
                  *model::converters::stringToJson(bytesToString(bytes)))));
        };
        boost::for_each(
            result | boost::adaptors::transformed([&block](const auto &x) {
              return x.at("index").template as<size_t>();
            }),
            [&](auto x) {
              subscriber.on_next(PostgresBlockQuery::wTransaction(
                  block->transactions().at(x)->copy()));
            });
      };
    }

    rxcpp::observable<BlockQuery::wTransaction>
    PostgresBlockQuery::getAccountTransactions(
        const shared_model::interface::types::AccountIdType &account_id) {
      return rxcpp::observable<>::create<wTransaction>(
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

    rxcpp::observable<BlockQuery::wTransaction>
    PostgresBlockQuery::getAccountAssetTransactions(
        const shared_model::interface::types::AccountIdType &account_id,
        const shared_model::interface::types::AssetIdType &asset_id) {
      return rxcpp::observable<>::create<wTransaction>([this,
                                                        account_id,
                                                        asset_id](
                                                           auto subscriber) {
        auto block_ids = this->getBlockIds(account_id);
        if (block_ids.empty()) {
          subscriber.on_completed();
          return;
        }

        for (const auto &block_id : block_ids) {
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

    rxcpp::observable<boost::optional<BlockQuery::wTransaction>>
    PostgresBlockQuery::getTransactions(
        const std::vector<shared_model::crypto::Hash> &tx_hashes) {
      return rxcpp::observable<>::create<boost::optional<wTransaction>>(
          [this, tx_hashes](auto subscriber) {
            std::for_each(tx_hashes.begin(),
                          tx_hashes.end(),
                          [that = this, &subscriber](auto tx_hash) {
                            subscriber.on_next(that->getTxByHashSync(tx_hash));
                          });
            subscriber.on_completed();
          });
    }

    boost::optional<BlockQuery::wTransaction>
    PostgresBlockQuery::getTxByHashSync(
        const shared_model::crypto::Hash &hash) {
      return getBlockId(hash) |
          [this](auto blockId) { return block_store_.get(blockId); } |
          [this](auto bytes) {
            // TODO IR-975 victordrobny 12.02.2018 convert directly to
            // shared_model::proto::Block after FlatFile will be reworked to new
            // model
            return model::converters::stringToJson(bytesToString(bytes));
          }
      | [&](const auto &json) { return serializer_.deserialize(json); } |
          [](const auto &block) {
            return boost::optional<shared_model::proto::Block>(
                shared_model::proto::from_old(block));
          }
      | [&](const auto &block) {
          boost::optional<PostgresBlockQuery::wTransaction> result;
          auto it =
              std::find_if(block.transactions().begin(),
                           block.transactions().end(),
                           [&hash](auto tx) { return tx->hash() == hash; });
          if (it != block.transactions().end()) {
            result = boost::optional<PostgresBlockQuery::wTransaction>(
                PostgresBlockQuery::wTransaction((*it)->copy()));
          }
          return result;
        };
    }

  }  // namespace ametsuchi
}  // namespace iroha

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

#include "converters/protobuf/json_proto_converter.hpp"

namespace iroha {
  namespace ametsuchi {

    PostgresBlockQuery::PostgresBlockQuery(soci::session &sql,
                                           KeyValueStorage &file_store)
        : sql_(sql),
          block_store_(file_store),
          log_(logger::log("PostgresBlockIndex")) {}

    PostgresBlockQuery::PostgresBlockQuery(
        std::unique_ptr<soci::session> sql_ptr, KeyValueStorage &file_store)
        : sql_ptr_(std::move(sql_ptr)),
          sql_(*sql_ptr_),
          block_store_(file_store),
          log_(logger::log("PostgresBlockIndex")) {}

    rxcpp::observable<BlockQuery::wBlock> PostgresBlockQuery::getBlocks(
        shared_model::interface::types::HeightType height, uint32_t count) {
      shared_model::interface::types::HeightType last_id =
          block_store_.last_id();
      auto to = std::min(last_id, height + count - 1);
      if (height > to or count == 0) {
        return rxcpp::observable<>::empty<wBlock>();
      }
      return rxcpp::observable<>::range(height, to)
          .flat_map([this](const auto &i) {
            return rxcpp::observable<>::create<PostgresBlockQuery::wBlock>(
                [i, this](const auto &block_subscriber) {
                  block_store_.get(i) | [](const auto &bytes) {
                    return shared_model::converters::protobuf::jsonToModel<
                        shared_model::proto::Block>(bytesToString(bytes));
                  } | [&block_subscriber](auto &&block) {
                    block_subscriber.on_next(
                        std::make_shared<shared_model::proto::Block>(
                            std::move(block)));
                  };
                  block_subscriber.on_completed();
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
      std::vector<shared_model::interface::types::HeightType> result;
      soci::indicator ind;
      std::string row;
      soci::statement st =
          (sql_.prepare << "SELECT DISTINCT height FROM height_by_account_set "
                           "WHERE account_id = :id",
           soci::into(row, ind),
           soci::use(account_id));
      st.execute();

      processSoci(st, ind, row, [&result](std::string &r) {
        result.push_back(std::stoull(r));
      });
      return result;
    }

    boost::optional<shared_model::interface::types::HeightType>
    PostgresBlockQuery::getBlockId(const shared_model::crypto::Hash &hash) {
      boost::optional<uint64_t> blockId = boost::none;
      boost::optional<std::string> block_str;
      auto hash_str = hash.hex();

      sql_ << "SELECT height FROM height_by_hash WHERE hash = :hash",
          soci::into(block_str), soci::use(hash_str);
      if (block_str) {
        blockId = std::stoull(block_str.get());
      } else {
        log_->info("No block with transaction {}", hash.toString());
      }
      return blockId;
    }

    std::function<void(std::vector<std::string> &result)>
    PostgresBlockQuery::callback(
        const rxcpp::subscriber<wTransaction> &subscriber, uint64_t block_id) {
      return [this, &subscriber, block_id](std::vector<std::string> &result) {
        auto block = block_store_.get(block_id) | [](const auto &bytes) {
          return shared_model::converters::protobuf::jsonToModel<
              shared_model::proto::Block>(bytesToString(bytes));
        };
        if (not block) {
          log_->error("error while converting from JSON");
          return;
        }

        boost::for_each(
            result | boost::adaptors::transformed([](const auto &x) {
              std::istringstream iss(x);
              size_t size;
              iss >> size;
              return size;
            }),
            [&](const auto &x) {
              subscriber.on_next(PostgresBlockQuery::wTransaction(
                  clone(block->transactions()[x])));
            });
      };
    }

    rxcpp::observable<BlockQuery::wTransaction>
    PostgresBlockQuery::getAccountTransactions(
        const shared_model::interface::types::AccountIdType &account_id) {
      return rxcpp::observable<>::create<wTransaction>(
          [this, account_id](const auto &subscriber) {
            auto block_ids = this->getBlockIds(account_id);
            if (block_ids.empty()) {
              subscriber.on_completed();
              return;
            }

            for (const auto &block_id : block_ids) {
              std::vector<std::string> index;
              soci::indicator ind;
              std::string row;
              soci::statement st =
                  (sql_.prepare
                       << "SELECT DISTINCT index FROM index_by_creator_height "
                          "WHERE creator_id = :id AND height = :height",
                   soci::into(row, ind),
                   soci::use(account_id),
                   soci::use(block_id));
              st.execute();

              processSoci(st, ind, row, [&index](std::string &r) {
                index.push_back(r);
              });
              this->callback(subscriber, block_id)(index);
            }
            subscriber.on_completed();
          });
    }

    rxcpp::observable<BlockQuery::wTransaction>
    PostgresBlockQuery::getAccountAssetTransactions(
        const shared_model::interface::types::AccountIdType &account_id,
        const shared_model::interface::types::AssetIdType &asset_id) {
      return rxcpp::observable<>::create<wTransaction>(
          [this, account_id, asset_id](auto subscriber) {
            auto block_ids = this->getBlockIds(account_id);
            if (block_ids.empty()) {
              subscriber.on_completed();
              return;
            }

            for (const auto &block_id : block_ids) {
              std::vector<std::string> index;
              soci::indicator ind;
              std::string row;
              soci::statement st =
                  (sql_.prepare
                       << "SELECT DISTINCT index FROM index_by_id_height_asset "
                          "WHERE id = :id AND height = :height AND asset_id = "
                          ":asset_id",
                   soci::into(row, ind),
                   soci::use(account_id),
                   soci::use(block_id),
                   soci::use(asset_id));
              st.execute();

              processSoci(st, ind, row, [&index](std::string &r) {
                index.push_back(r);
              });
              this->callback(subscriber, block_id)(index);
            }
            subscriber.on_completed();
          });
    }

    rxcpp::observable<boost::optional<BlockQuery::wTransaction>>
    PostgresBlockQuery::getTransactions(
        const std::vector<shared_model::crypto::Hash> &tx_hashes) {
      return rxcpp::observable<>::create<boost::optional<wTransaction>>(
          [this, tx_hashes](const auto &subscriber) {
            std::for_each(tx_hashes.begin(),
                          tx_hashes.end(),
                          [that = this, &subscriber](const auto &tx_hash) {
                            subscriber.on_next(that->getTxByHashSync(tx_hash));
                          });
            subscriber.on_completed();
          });
    }

    boost::optional<BlockQuery::wTransaction>
    PostgresBlockQuery::getTxByHashSync(
        const shared_model::crypto::Hash &hash) {
      auto block = getBlockId(hash) | [this](const auto &block_id) {
        return block_store_.get(block_id);
      } | [](const auto &bytes) {
        return shared_model::converters::protobuf::jsonToModel<
            shared_model::proto::Block>(bytesToString(bytes));
      };
      if (not block) {
        log_->error("error while converting from JSON");
        return boost::none;
      }

      boost::optional<PostgresBlockQuery::wTransaction> result;
      auto it =
          std::find_if(block->transactions().begin(),
                       block->transactions().end(),
                       [&hash](const auto &tx) { return tx.hash() == hash; });
      if (it != block->transactions().end()) {
        result = boost::optional<PostgresBlockQuery::wTransaction>(
            PostgresBlockQuery::wTransaction(clone(*it)));
      }
      return result;
    }

    bool PostgresBlockQuery::hasTxWithHash(
        const shared_model::crypto::Hash &hash) {
      return getBlockId(hash) != boost::none;
    }

    uint32_t PostgresBlockQuery::getTopBlockHeight() {
      return block_store_.last_id();
    }

    expected::Result<BlockQuery::wBlock, std::string>
    PostgresBlockQuery::getTopBlock() {
      // TODO 18/06/18 Akvinikym: add dependency injection IR-937 IR-1040
      auto block =
          block_store_.get(block_store_.last_id()) | [](const auto &bytes) {
            return shared_model::converters::protobuf::jsonToModel<
                shared_model::proto::Block>(bytesToString(bytes));
          };
      if (not block) {
        return expected::makeError("error while fetching the last block");
      }
      return expected::makeValue(std::make_shared<shared_model::proto::Block>(
          std::move(block.value())));
    }

  }  // namespace ametsuchi
}  // namespace iroha

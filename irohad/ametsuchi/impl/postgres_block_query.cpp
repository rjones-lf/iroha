/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ametsuchi/impl/postgres_block_query.hpp"

#include <boost/format.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/algorithm/for_each.hpp>

#include "ametsuchi/impl/soci_utils.hpp"
#include "common/byteutils.hpp"

namespace iroha {
  namespace ametsuchi {
    PostgresBlockQuery::PostgresBlockQuery(
        soci::session &sql,
        KeyValueStorage &file_store,
        std::shared_ptr<shared_model::interface::BlockJsonDeserializer>
            converter)
        : sql_(sql),
          block_store_(file_store),
          converter_(std::move(converter)),
          log_(logger::log("PostgresBlockQuery")) {}

    PostgresBlockQuery::PostgresBlockQuery(
        std::unique_ptr<soci::session> sql,
        KeyValueStorage &file_store,
        std::shared_ptr<shared_model::interface::BlockJsonDeserializer>
            converter)
        : psql_(std::move(sql)),
          sql_(*psql_),
          block_store_(file_store),
          converter_(std::move(converter)),
          log_(logger::log("PostgresBlockQuery")) {}

    std::vector<BlockQuery::wBlock> PostgresBlockQuery::getBlocks(
        shared_model::interface::types::HeightType height, uint32_t count) {
      shared_model::interface::types::HeightType last_id =
          block_store_.last_id();
      auto to = std::min(last_id, height + count - 1);
      std::vector<BlockQuery::wBlock> result;
      if (height > to or count == 0) {
        return result;
      }
      for (auto i = height; i <= to; i++) {
        auto block = getBlock(i);
        block.match(
            [&result](
                expected::Value<std::unique_ptr<shared_model::interface::Block>>
                    &v) { result.emplace_back(std::move(v.value)); },
            [this](const expected::Error<std::string> &e) {
              log_->error(e.error);
            });
      }
      return result;
    }

    std::vector<BlockQuery::wBlock> PostgresBlockQuery::getBlocksFrom(
        shared_model::interface::types::HeightType height) {
      return getBlocks(height, block_store_.last_id());
    }

    std::vector<BlockQuery::wBlock> PostgresBlockQuery::getTopBlocks(
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

      sql_ << "SELECT height FROM position_by_hash WHERE hash = :hash",
          soci::into(block_str), soci::use(hash_str);
      if (block_str) {
        blockId = std::stoull(block_str.get());
      } else {
        log_->info("No block with transaction {}", hash.toString());
      }
      return blockId;
    }

    std::function<void(std::vector<std::string> &result)>
    PostgresBlockQuery::callback(std::vector<wTransaction> &blocks,
                                 uint64_t block_id) {
      return [this, &blocks, block_id](std::vector<std::string> &result) {
        auto block = getBlock(block_id);
        block.match(
            [&result, &blocks](
                expected::Value<std::unique_ptr<shared_model::interface::Block>>
                    &v) {
              boost::for_each(
                  result | boost::adaptors::transformed([](const auto &x) {
                    std::istringstream iss(x);
                    size_t size;
                    iss >> size;
                    return size;
                  }),
                  [&](const auto &x) {
                    blocks.emplace_back(clone(v.value->transactions()[x]));
                  });
            },
            [this](const expected::Error<std::string> &e) {
              log_->error(e.error);
            });
      };
    }

    std::vector<BlockQuery::wTransaction>
    PostgresBlockQuery::getAccountTransactions(
        const shared_model::interface::types::AccountIdType &account_id) {
      std::vector<BlockQuery::wTransaction> result;
      auto block_ids = this->getBlockIds(account_id);
      if (block_ids.empty()) {
        return result;
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

        processSoci(
            st, ind, row, [&index](std::string &r) { index.push_back(r); });
        this->callback(result, block_id)(index);
      }
      return result;
    }

    std::vector<BlockQuery::wTransaction>
    PostgresBlockQuery::getAccountAssetTransactions(
        const shared_model::interface::types::AccountIdType &account_id,
        const shared_model::interface::types::AssetIdType &asset_id) {
      std::vector<BlockQuery::wTransaction> result;
      auto block_ids = this->getBlockIds(account_id);
      if (block_ids.empty()) {
        return result;
      }

      for (const auto &block_id : block_ids) {
        std::vector<std::string> index;
        soci::indicator ind;
        std::string row;
        soci::statement st =
            (sql_.prepare
                 << "SELECT DISTINCT index FROM position_by_account_asset "
                    "WHERE account_id = :id AND height = :height "
                    "AND asset_id = :asset_id",
             soci::into(row, ind),
             soci::use(account_id),
             soci::use(block_id),
             soci::use(asset_id));
        st.execute();

        processSoci(
            st, ind, row, [&index](std::string &r) { index.push_back(r); });
        this->callback(result, block_id)(index);
      }
      return result;
    }

    std::vector<boost::optional<BlockQuery::wTransaction>>
    PostgresBlockQuery::getTransactions(
        const std::vector<shared_model::crypto::Hash> &tx_hashes) {
      std::vector<boost::optional<BlockQuery::wTransaction>> result;
      std::for_each(tx_hashes.begin(),
                    tx_hashes.end(),
                    [this, &result](const auto &tx_hash) {
                      result.push_back(this->getTxByHashSync(tx_hash));
                    });
      return result;
    }

    boost::optional<BlockQuery::wTransaction>
    PostgresBlockQuery::getTxByHashSync(
        const shared_model::crypto::Hash &hash) {
      return getBlockId(hash) |
          [this](const auto &block_id) {
            auto result = this->getBlock(block_id);
            return result.match(
                [](expected::Value<
                    std::unique_ptr<shared_model::interface::Block>> &v)
                    -> boost::optional<
                        std::unique_ptr<shared_model::interface::Block>> {
                  return std::move(v.value);
                },
                [this](const expected::Error<std::string> &e)
                    -> boost::optional<
                        std::unique_ptr<shared_model::interface::Block>> {
                  log_->error(e.error);
                  return boost::none;
                });
          }
      | [&hash, this](const auto &block) {
          auto it = std::find_if(
              block->transactions().begin(),
              block->transactions().end(),
              [&hash](const auto &tx) { return tx.hash() == hash; });
          if (it != block->transactions().end()) {
            return boost::make_optional<PostgresBlockQuery::wTransaction>(
                clone(*it));
          } else {
            log_->error("Failed to find transaction {} in block {}",
                        hash.hex(),
                        block->height());
            // return type specification for lambda breaks formatting.
            // That is why optional is constructed explicitly
            return boost::optional<PostgresBlockQuery::wTransaction>(
                boost::none);
          }
        };
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
      return getBlock(block_store_.last_id())
          .match(
              [](expected::Value<
                  std::unique_ptr<shared_model::interface::Block>> &v)
                  -> expected::Result<BlockQuery::wBlock, std::string> {
                return expected::makeValue<BlockQuery::wBlock>(
                    std::move(v.value));
              },
              [](expected::Error<std::string> &e)
                  -> expected::Result<BlockQuery::wBlock, std::string> {
                return expected::makeError(std::move(e.error));
              });
    }

    expected::Result<std::unique_ptr<shared_model::interface::Block>,
                     std::string>
    PostgresBlockQuery::getBlock(
        shared_model::interface::types::HeightType id) const {
      auto serialized_block = block_store_.get(id);
      if (not serialized_block) {
        auto error = boost::format("Failed to retrieve block with id %d") % id;
        return expected::makeError(error.str());
      }
      return converter_->deserialize(bytesToString(*serialized_block));
    }
  }  // namespace ametsuchi
}  // namespace iroha

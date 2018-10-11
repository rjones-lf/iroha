/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ametsuchi/impl/postgres_block_index.hpp"

#include <numeric>

#include <boost/format.hpp>
#include <boost/range/adaptor/indexed.hpp>
#include "common/types.hpp"
#include "common/visitor.hpp"
#include "interfaces/commands/command.hpp"
#include "interfaces/commands/transfer_asset.hpp"
#include "interfaces/common_objects/types.hpp"
#include "interfaces/iroha_internal/block.hpp"

namespace {
  // Return transfer asset if command contains it
  boost::optional<const shared_model::interface::TransferAsset &>
  getTransferAsset(const shared_model::interface::Command &cmd) noexcept {
    using ReturnType =
        boost::optional<const shared_model::interface::TransferAsset &>;
    return iroha::visit_in_place(
        cmd.get(),
        [](const shared_model::interface::TransferAsset &c) {
          return ReturnType(c);
        },
        [&](const auto &command) -> ReturnType { return boost::none; });
  }

  // tx hash -> block where hash is stored
  std::string makeHashIndex(
      const shared_model::interface::types::HashType &hash,
      const std::string &height) {
    boost::format base(
        "INSERT INTO height_by_hash(hash, height) VALUES ('%s', "
        "'%s');");
    return (base % hash.hex() % height).str();
  }

  // make index account_id:height -> list of tx indexes
  // (where tx is placed in the block)
  std::string makeCreatorHeightIndex(
      const shared_model::interface::types::AccountIdType creator,
      const std::string &height,
      const std::string &tx_index) {
    boost::format base(
        "INSERT INTO index_by_creator_height(creator_id, height, index) VALUES "
        "('%s', '%s', '%s');");
    return (base % creator % height % tx_index).str();
  }

  // Make index account_id -> list of blocks where his txs exist
  std::string makeAccountHeightIndex(const std::string &account_id,
                                     const std::string &height) {
    boost::format base(
        "INSERT INTO height_by_account_set(account_id, "
        "height) VALUES "
        "('%s', '%s');");
    return (base % account_id % height).str();
  }

  // Collect all assets belonging to creator, sender, and receiver
  // to make account_id:height:asset_id -> list of tx indexes
  // for transfer asset in command
  std::string makeAccountAssetIndex(
      const shared_model::interface::types::AccountIdType &account_id,
      const std::string &height,
      const std::string &index,
      const shared_model::interface::Transaction::CommandsType &commands) {
    return std::accumulate(
        commands.begin(),
        commands.end(),
        std::string{},
        [&](auto query, const auto &cmd) {
          auto transfer = getTransferAsset(cmd);
          if (not transfer) {
            return query;
          }
          const auto &src_id = transfer.value().srcAccountId();
          const auto &dest_id = transfer.value().destAccountId();

          query += makeAccountHeightIndex(src_id, height);
          query += makeAccountHeightIndex(dest_id, height);

          const auto ids = {account_id, src_id, dest_id};
          const auto &asset_id = transfer.value().assetId();
          // flat map accounts to unindexed keys
          for (const auto &id : ids) {
            boost::format base(
                "INSERT INTO index_by_id_height_asset(id, "
                "height, asset_id, "
                "index) "
                "VALUES ('%s', '%s', '%s', '%s');");
            query += (base % id % height % asset_id % index).str();
          }
          return query;
        });
  }
}  // namespace

namespace iroha {
  namespace ametsuchi {
    PostgresBlockIndex::PostgresBlockIndex(soci::session &sql)
        : sql_(sql), log_(logger::log("PostgresBlockIndex")) {}

    void PostgresBlockIndex::index(
        const shared_model::interface::Block &block) {
      const auto &height = std::to_string(block.height());
      auto indexed_txs = block.transactions() | boost::adaptors::indexed(0);
      std::string index_query = std::accumulate(
          indexed_txs.begin(),
          indexed_txs.end(),
          std::string{},
          [&height](auto query, const auto &tx) {
            const auto &creator_id = tx.value().creatorAccountId();
            const auto index = std::to_string(tx.index());

            query += makeAccountHeightIndex(creator_id, height);
            query += makeAccountAssetIndex(
                creator_id, height, index, tx.value().commands());
            query += makeHashIndex(tx.value().hash(), height);
            query += makeCreatorHeightIndex(creator_id, height, index);
            return query;
          });
      sql_ << index_query;
    }
  }  // namespace ametsuchi
}  // namespace iroha

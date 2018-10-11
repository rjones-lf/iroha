/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ametsuchi/impl/postgres_block_index.hpp"

#include <numeric>
#include <unordered_set>

#include <boost/format.hpp>
#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/adaptor/indexed.hpp>
#include <boost/range/algorithm/for_each.hpp>
#include <boost/range/numeric.hpp>
#include "common/types.hpp"
#include "common/visitor.hpp"
#include "cryptography/ed25519_sha3_impl/internal/sha3_hash.hpp"
#include "interfaces/commands/command.hpp"
#include "interfaces/commands/transfer_asset.hpp"
#include "interfaces/common_objects/types.hpp"
#include "interfaces/iroha_internal/block.hpp"

namespace {
  // tx hash -> block where hash is stored
  std::string makeHashIndexQuery(
      const shared_model::interface::types::HashType &hash,
      const std::string &height) {
    boost::format base(
        "INSERT INTO height_by_hash(hash, height) VALUES ('%s', "
        "'%s');");
    return (base % hash.hex() % height).str();
  }

  // make index account_id:height -> list of tx indexes
  // (where tx is placed in the block)
  std::string makeCreatorHeightIndexQuery(
      const shared_model::interface::types::AccountIdType creator,
      const std::string &height,
      const std::string &tx_index) {
    boost::format base(
        "INSERT INTO index_by_creator_height(creator_id, height, index) VALUES "
        "('%s', '%s', '%s');");
    return (base % creator % height % tx_index).str();
  }

  // Make index account_id -> list of blocks where his txs exist
  std::string makeAccountHeightIndexQuery(const std::string &account_id,
                                          const std::string &height) {
    boost::format base(
        "INSERT INTO height_by_account_set(account_id, "
        "height) VALUES "
        "('%s', '%s');");

    return (base % account_id % height).str();
  }

  // Collect all assets belonging to creator, sender, and receiver
  // to make account_id:height:asset_id -> list of tx indexes
  std::string makeAccountAssetIndexQuery(
      const shared_model::interface::types::AccountIdType &account_id,
      const std::string &height,
      const std::string &index,
      const shared_model::interface::Transaction::CommandsType &commands) {
    return std::accumulate(
        commands.begin(),
        commands.end(),
        std::string{},
        [&](auto query, const auto &cmd) {
          iroha::visit_in_place(
              cmd.get(),
              [&](const shared_model::interface::TransferAsset &command) {
                query +=
                    makeAccountHeightIndexQuery(command.srcAccountId(), height);
                query += makeAccountHeightIndexQuery(command.destAccountId(),
                                                     height);

                auto ids = {account_id,
                            command.srcAccountId(),
                            command.destAccountId()};
                auto &asset_id = command.assetId();
                // flat map accounts to unindexed keys
                boost::for_each(ids, [&](const auto &id) {
                  boost::format base(
                      "INSERT INTO index_by_id_height_asset(id, "
                      "height, asset_id, "
                      "index) "
                      "VALUES ('%s', '%s', '%s', '%s');");
                  query += (base % id % height % asset_id % index).str();
                });
              },
              [&](const auto &command) {});
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

            query += makeAccountHeightIndexQuery(creator_id, height);
            query += makeAccountAssetIndexQuery(
                creator_id, height, index, tx.value().commands());
            query += makeHashIndexQuery(tx.value().hash(), height);
            query += makeCreatorHeightIndexQuery(creator_id, height, index);
            return query;
          });
      sql_ << index_query;
    }
  }  // namespace ametsuchi
}  // namespace iroha

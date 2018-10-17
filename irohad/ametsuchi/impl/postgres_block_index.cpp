/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ametsuchi/impl/postgres_block_index.hpp"

#include <unordered_set>

#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/adaptor/indexed.hpp>
#include <boost/range/algorithm/for_each.hpp>
#include <boost/range/numeric.hpp>
#include "common/types.hpp"
#include "common/visitor.hpp"
#include "cryptography/ed25519_sha3_impl/internal/sha3_hash.hpp"
#include "interfaces/commands/command_variant.hpp"
#include "interfaces/commands/transfer_asset.hpp"
#include "interfaces/iroha_internal/block.hpp"

namespace iroha {
  namespace ametsuchi {

    bool execute(soci::statement &st) {
      st.define_and_bind();
      try {
        st.execute(true);
        return true;
      } catch (const std::exception &e) {
        return false;
      }
    }

    PostgresBlockIndex::PostgresBlockIndex(soci::session &sql)
        : sql_(sql), log_(logger::log("PostgresBlockIndex")) {}

    auto PostgresBlockIndex::indexAccountIdHeight(const std::string &account_id,
                                                  const std::string &height) {
      soci::statement st =
          (sql_.prepare << "INSERT INTO height_by_account_set(account_id, "
                           "height) VALUES "
                           "(:id, :height)",
           soci::use(account_id),
           soci::use(height));
      return execute(st);
    }

    auto PostgresBlockIndex::indexAccountAssets(
        const std::string &account_id,
        const std::string &height,
        const std::string &index,
        const shared_model::interface::Transaction::CommandsType &commands) {
      // flat map abstract commands to transfers

      return std::accumulate(
          commands.begin(),
          commands.end(),
          true,
          [&](auto &status, const auto &cmd) {
            return visit_in_place(
                cmd.get(),
                [&](const shared_model::interface::TransferAsset &command) {
                  status &=
                      this->indexAccountIdHeight(command.srcAccountId(), height)
                      & this->indexAccountIdHeight(command.destAccountId(),
                                                   height);

                  auto ids = {account_id,
                              command.srcAccountId(),
                              command.destAccountId()};
                  auto &asset_id = command.assetId();
                  // flat map accounts to unindexed keys
                  boost::for_each(ids, [&](const auto &id) {
                    soci::statement st =
                        (sql_.prepare
                             << "INSERT INTO index_by_id_height_asset(id, "
                                "height, asset_id, "
                                "index) "
                                "VALUES (:id, :height, :asset_id, :index)",
                         soci::use(id),
                         soci::use(height),
                         soci::use(asset_id),
                         soci::use(index));
                    status &= execute(st);
                  });
                  return status;
                },
                [&](const auto &command) { return true; });
          });
    }

    void PostgresBlockIndex::index(
        const shared_model::interface::Block &block) {
      const auto &height = std::to_string(block.height());
      boost::for_each(
          block.transactions() | boost::adaptors::indexed(0),
          [&](const auto &tx) {
            const auto &creator_id = tx.value().creatorAccountId();
            const auto &hash = tx.value().hash().hex();
            const auto &index = std::to_string(tx.index());

            // tx hash -> block where hash is stored
            soci::statement st =
                (sql_.prepare << "INSERT INTO height_by_hash(hash, height) "
                                 "VALUES (:hash, "
                                 ":height)",
                 soci::use(hash),
                 soci::use(height));
            execute(st);

            this->indexAccountIdHeight(creator_id, height);

            // to make index account_id:height -> list of tx indexes
            // (where tx is placed in the block)
            st = (sql_.prepare
                      << "INSERT INTO index_by_creator_height(creator_id, "
                         "height, index) "
                         "VALUES (:id, :height, :index)",
                  soci::use(creator_id),
                  soci::use(height),
                  soci::use(index));
            execute(st);

            this->indexAccountAssets(
                creator_id, height, index, tx.value().commands());
          });
    }
  }  // namespace ametsuchi
}  // namespace iroha

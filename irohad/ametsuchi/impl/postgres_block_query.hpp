/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_POSTGRES_FLAT_BLOCK_QUERY_HPP
#define IROHA_POSTGRES_FLAT_BLOCK_QUERY_HPP

#include "ametsuchi/block_query.hpp"

#include <soci/soci.h>
#include <boost/optional.hpp>
#include "ametsuchi/impl/flat_file/flat_file.hpp"
#include "interfaces/iroha_internal/block_json_deserializer.hpp"
#include "logger/logger.hpp"

namespace iroha {
  namespace ametsuchi {

    class FlatFile;

    /**
     * Class which implements BlockQuery with a Postgres backend.
     */
    class PostgresBlockQuery : public BlockQuery {
     public:
      PostgresBlockQuery(
          soci::session &sql,
          KeyValueStorage &file_store,
          std::shared_ptr<shared_model::interface::BlockJsonDeserializer>
              converter);

      PostgresBlockQuery(
          std::unique_ptr<soci::session> sql,
          KeyValueStorage &file_store,
          std::shared_ptr<shared_model::interface::BlockJsonDeserializer>
              converter);

      std::vector<wTransaction> getAccountTransactions(
          const shared_model::interface::types::AccountIdType &account_id)
          override;

      std::vector<wTransaction> getAccountAssetTransactions(
          const shared_model::interface::types::AccountIdType &account_id,
          const shared_model::interface::types::AssetIdType &asset_id) override;

      std::vector<boost::optional<wTransaction>> getTransactions(
          const std::vector<shared_model::crypto::Hash> &tx_hashes) override;

      boost::optional<wTransaction> getTxByHashSync(
          const shared_model::crypto::Hash &hash) override;

      std::vector<wBlock> getBlocks(
          shared_model::interface::types::HeightType height,
          uint32_t count) override;

      std::vector<wBlock> getBlocksFrom(
          shared_model::interface::types::HeightType height) override;

      std::vector<wBlock> getTopBlocks(uint32_t count) override;

      uint32_t getTopBlockHeight() override;

      boost::optional<TxCacheStatusType> checkTxPresence(
          const shared_model::crypto::Hash &hash) override;

      expected::Result<wBlock, std::string> getTopBlock() override;

     private:
      /**
       * Returns all blocks' ids containing given account id
       * @param account_id
       * @return vector of block ids
       */
      std::vector<shared_model::interface::types::HeightType> getBlockIds(
          const shared_model::interface::types::AccountIdType &account_id);
      /**
       * Returns block id which contains transaction with a given hash
       * @param hash - hash of transaction
       * @return block id or boost::none
       */
      boost::optional<shared_model::interface::types::HeightType> getBlockId(
          const shared_model::crypto::Hash &hash);

      /**
       * creates callback to lrange query to Postgres to supply result to
       * subscriber s
       * @param s
       * @param block_id
       * @return
       */
      std::function<void(std::vector<std::string> &result)> callback(
          std::vector<wTransaction> &s, uint64_t block_id);

      /**
       * Retrieve block with given id block storage
       * @param id - height of a block to retrieve
       * @return block with given height
       */
      expected::Result<std::unique_ptr<shared_model::interface::Block>,
                       std::string>
      getBlock(shared_model::interface::types::HeightType id) const;

      std::unique_ptr<soci::session> psql_;
      soci::session &sql_;

      KeyValueStorage &block_store_;
      std::shared_ptr<shared_model::interface::BlockJsonDeserializer>
          converter_;

      logger::Logger log_;
    };
  }  // namespace ametsuchi
}  // namespace iroha

#endif  // IROHA_POSTGRES_FLAT_BLOCK_QUERY_HPP

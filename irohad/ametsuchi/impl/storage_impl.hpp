/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_STORAGE_IMPL_HPP
#define IROHA_STORAGE_IMPL_HPP

#include "ametsuchi/storage.hpp"

#include <cmath>
#include <shared_mutex>

#include <soci/soci.h>
#include <boost/optional.hpp>

#include "ametsuchi/impl/postgres_options.hpp"
#include "ametsuchi/key_value_storage.hpp"
#include "interfaces/common_objects/common_objects_factory.hpp"
#include "interfaces/iroha_internal/block_json_converter.hpp"
#include "logger/logger.hpp"

namespace iroha {
  namespace ametsuchi {

    class FlatFile;

    struct ConnectionContext {
      explicit ConnectionContext(std::unique_ptr<KeyValueStorage> block_store);

      std::unique_ptr<KeyValueStorage> block_store;
    };

    class StorageImpl : public Storage {
     protected:
      static expected::Result<bool, std::string> createDatabaseIfNotExist(
          const std::string &dbname,
          const std::string &options_str_without_dbname);

      static expected::Result<ConnectionContext, std::string> initConnections(
          std::string block_store_dir);

      static expected::Result<std::shared_ptr<soci::connection_pool>,
                              std::string>
      initPostgresConnection(std::string &options_str, size_t pool_size);

     public:
      static expected::Result<std::shared_ptr<StorageImpl>, std::string> create(
          std::string block_store_dir,
          std::string postgres_connection,
          std::shared_ptr<shared_model::interface::CommonObjectsFactory>
              factory,
          std::shared_ptr<shared_model::interface::BlockJsonConverter>
              converter,
          size_t pool_size = 10);

      expected::Result<std::unique_ptr<TemporaryWsv>, std::string>
      createTemporaryWsv() override;

      expected::Result<std::unique_ptr<MutableStorage>, std::string>
      createMutableStorage() override;

      boost::optional<std::shared_ptr<PeerQuery>> createPeerQuery()
          const override;

      boost::optional<std::shared_ptr<BlockQuery>> createBlockQuery()
          const override;

      boost::optional<std::shared_ptr<OrderingServicePersistentState>>
      createOsPersistentState() const override;

      /**
       * Insert block without validation
       * @param blocks - block for insertion
       * @return true if all blocks are inserted
       */
      bool insertBlock(const shared_model::interface::Block &block) override;

      /**
       * Insert blocks without validation
       * @param blocks - collection of blocks for insertion
       * @return true if inserted
       */
      bool insertBlocks(
          const std::vector<std::shared_ptr<shared_model::interface::Block>>
              &blocks) override;

      void reset() override;

      void dropStorage() override;

      void freeConnections() override;

      void commit(std::unique_ptr<MutableStorage> mutableStorage) override;

      std::shared_ptr<WsvQuery> getWsvQuery() const override;

      std::shared_ptr<BlockQuery> getBlockQuery() const override;

      rxcpp::observable<std::shared_ptr<shared_model::interface::Block>>
      on_commit() override;

      ~StorageImpl() override;

     protected:
      StorageImpl(std::string block_store_dir,
                  PostgresOptions postgres_options,
                  std::unique_ptr<KeyValueStorage> block_store,
                  std::shared_ptr<soci::connection_pool> connection,
                  std::shared_ptr<shared_model::interface::CommonObjectsFactory>
                      factory,
                  std::shared_ptr<shared_model::interface::BlockJsonConverter>
                      converter,
                  size_t pool_size);

      /**
       * Folder with raw blocks
       */
      const std::string block_store_dir_;

      // db info
      const PostgresOptions postgres_options_;

     private:
      std::unique_ptr<KeyValueStorage> block_store_;

      std::shared_ptr<soci::connection_pool> connection_;

      std::shared_ptr<shared_model::interface::CommonObjectsFactory> factory_;

      rxcpp::subjects::subject<std::shared_ptr<shared_model::interface::Block>>
          notifier_;

      std::shared_ptr<shared_model::interface::BlockJsonConverter> converter_;

      logger::Logger log_;

      mutable std::shared_timed_mutex drop_mutex;

      size_t pool_size_;

     protected:
      static const std::string &drop_;
      static const std::string &reset_;
      static const std::string &init_;
    };
  }  // namespace ametsuchi
}  // namespace iroha

#endif  // IROHA_STORAGE_IMPL_HPP

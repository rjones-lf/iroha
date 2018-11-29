/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_MUTABLE_STORAGE_IMPL_HPP
#define IROHA_MUTABLE_STORAGE_IMPL_HPP

#include "ametsuchi/mutable_storage.hpp"

#include <map>

#include <soci/soci.h>
#include "ametsuchi/command_executor.hpp"
#include "interfaces/common_objects/common_objects_factory.hpp"
#include "logger/logger.hpp"

namespace iroha {
  namespace ametsuchi {
    class BlockIndex;
    class PostgresCommandExecutor;

    class MutableStorageImpl : public MutableStorage {
      friend class StorageImpl;

     public:
      MutableStorageImpl(
          shared_model::interface::types::HashType top_hash,
          std::shared_ptr<PostgresCommandExecutor> cmd_executor,
          std::unique_ptr<soci::session> sql,
          std::shared_ptr<shared_model::interface::CommonObjectsFactory>
              factory);

      bool apply(const shared_model::interface::Block &block) override;

      bool apply(rxcpp::observable<
                     std::shared_ptr<shared_model::interface::Block>> blocks,
                 MutableStoragePredicate predicate) override;

      ~MutableStorageImpl() override;

     private:
      /**
       * Performs a function inside savepoint, does a rollback if function
       * returned false, and removes the savepoint otherwise. Returns function
       * result
       */
      template <typename Function>
      bool withSavepoint(Function &&function);

      /**
       * Verifies whether the block is applicable using predicate, and applies
       * the block
       */
      bool apply(const shared_model::interface::Block &block,
                 MutableStoragePredicate predicate);

      shared_model::interface::types::HashType top_hash_;
      // ordered collection is used to enforce block insertion order in
      // StorageImpl::commit
      std::map<uint32_t, std::shared_ptr<shared_model::interface::Block>>
          block_store_;

      std::unique_ptr<soci::session> sql_;
      std::unique_ptr<PeerQuery> peer_query_;
      std::unique_ptr<BlockIndex> block_index_;
      std::shared_ptr<CommandExecutor> command_executor_;

      bool committed;

      logger::Logger log_;
    };
  }  // namespace ametsuchi
}  // namespace iroha

#endif  // IROHA_MUTABLE_STORAGE_IMPL_HPP

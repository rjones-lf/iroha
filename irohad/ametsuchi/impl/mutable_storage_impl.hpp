/**
 * Copyright Soramitsu Co., Ltd. 2017 All Rights Reserved.
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

#ifndef IROHA_MUTABLE_STORAGE_IMPL_HPP
#define IROHA_MUTABLE_STORAGE_IMPL_HPP

#include <soci/soci.h>
#include <map>

#include "ametsuchi/command_executor.hpp"
#include "ametsuchi/mutable_storage.hpp"
#include "interfaces/common_objects/common_objects_factory.hpp"
#include "logger/logger.hpp"

namespace iroha {

  namespace ametsuchi {

    class BlockIndex;
    class WsvCommand;

    class MutableStorageImpl : public MutableStorage {
      friend class StorageImpl;

     public:
      MutableStorageImpl(shared_model::interface::types::HashType top_hash,
                         std::unique_ptr<soci::session> sql,
                         std::shared_ptr<shared_model::interface::CommonObjectsFactory>
                         factory);
      bool check(const shared_model::interface::BlockVariant &block,
                 MutableStoragePredicateType<decltype(block)> function) override;

      bool apply(
          const shared_model::interface::Block &block,
          MutableStoragePredicateType<decltype(block)> function) override;

      ~MutableStorageImpl() override;

     private:
      shared_model::interface::types::HashType top_hash_;
      // ordered collection is used to enforce block insertion order in
      // StorageImpl::commit
      std::map<uint32_t, std::shared_ptr<shared_model::interface::Block>>
          block_store_;

      std::unique_ptr<soci::session> sql_;
      std::shared_ptr<WsvQuery> wsv_;
      std::shared_ptr<WsvCommand> executor_;
      std::unique_ptr<BlockIndex> block_index_;
      std::shared_ptr<CommandExecutor> command_executor_;

      bool committed;

      logger::Logger log_;
    };
  }  // namespace ametsuchi
}  // namespace iroha

#endif  // IROHA_MUTABLE_STORAGE_IMPL_HPP

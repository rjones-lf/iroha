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

#ifndef IROHA_MUTABLE_STORAGE_HPP
#define IROHA_MUTABLE_STORAGE_HPP

#include <functional>

#include "interfaces/common_objects/types.hpp"

namespace shared_model {
  namespace interface {
    class Block;
  }  // namespace interface
}  // namespace shared_model

namespace iroha {
  namespace ametsuchi {

    class WsvQuery;
    class PeerQuery;

    /**
     * Mutable storage is used apply blocks to the storage.
     * Allows to query the world state view, transactions, and blocks.
     */
    class MutableStorage {
     public:
      /**
       * Predicate type checking block
       */
      using MutableStoragePredicate =
          std::function<bool(const shared_model::interface::Block &,
                             PeerQuery &,
                             const shared_model::interface::types::HashType &)>;

      /**
       * Checks if block satisfies predicated
       * @param block block to be checked
       * @param predicate function returning true if predicate satisfied and
       * false otherwise
       * @return result of predicate
       */
      virtual bool check(const shared_model::interface::Block &block,
                         MutableStoragePredicate function) = 0;

      /**
       * Applies a block to current mutable state
       * using logic specified in function
       * @param block Block to be applied
       * @param function Function that specifies the logic used to apply the
       * block
       * Function parameters:
       *  - Block @see block
       *  - WsvQuery - world state view query interface for mutable storage
       *  - hash256_t - hash of top block in blockchain
       * Function returns true if the block is successfully applied, false
       * otherwise.
       * @return True if block was successfully applied, false otherwise.
       */
      virtual bool apply(const shared_model::interface::Block &block) = 0;

      virtual ~MutableStorage() = default;
    };

  }  // namespace ametsuchi
}  // namespace iroha

#endif  // IROHA_MUTABLE_STORAGE_HPP

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

#ifndef IROHA_AMETSUCHI_H
#define IROHA_AMETSUCHI_H

#include <vector>
#include "ametsuchi/mutable_factory.hpp"
#include "ametsuchi/temporary_factory.hpp"
#include "common/result.hpp"

namespace iroha {

  namespace model {
    struct Block;
  }

  namespace ametsuchi {

    class BlockQuery;
    class WsvQuery;

    /**
     * Storage interface, which allows queries on current committed state, and
     * creation of state which can be mutated with blocks and transactions
     */
    class Storage : public TemporaryFactory, public MutableFactory {
     public:
      virtual std::shared_ptr<WsvQuery> getWsvQuery() const = 0;

      virtual std::shared_ptr<BlockQuery> getBlockQuery() const = 0;

      /**
       * Raw insertion of blocks without validation
       * @param block - block for insertion
       * @return true if inserted
       */
      virtual bool insertBlock(model::Block block) = 0;

      /**
       * Raw insertion of blocks without validation
       * @param blocks - collection of blocks for insertion
       * @return true if inserted
       */
      virtual bool insertBlocks(const std::vector<model::Block> &blocks) = 0;

      /**
       * Remove all information from ledger
       */
      virtual void dropStorage() = 0;

      virtual ~Storage() = default;
    };

  }  // namespace ametsuchi

}  // namespace iroha

#endif  // IROHA_AMETSUCHI_H

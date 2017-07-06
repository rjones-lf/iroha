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

#include <ametsuchi/block_query.hpp>
#include <ametsuchi/mutable_storage.hpp>
#include <ametsuchi/wsv_query.hpp>
#include <ametsuchi/temporary_wsv.hpp>

namespace iroha {

  namespace ametsuchi {

    /**
     * Storage interface, which allows queries on current committed state, and
     * creation of state which can be mutated with blocks and transactions
     */
    class Ametsuchi : public WsvQuery, public BlockQuery {
     public:

      /**
       * Creates a temporary world state view from the current state.
       * Temporary state will be not committed and will be erased on destructor
       * call.
       * Temporary state might be used for transaction validation.
       * @return Created temporary wsv
       */
      virtual std::unique_ptr<TemporaryWsv> createTemporaryWsv() = 0;

      /**
       * Creates a mutable storage from the current state.
       * Mutable storage is the only way to commit the block to the ledger.
       * @return Created mutable storage
       */
      virtual std::unique_ptr<MutableStorage> createMutableStorage() = 0;

      /**
       * Commit mutable storage to Ametsuchi.
       * This transforms Ametsuchi to the new state consistent with
       * MutableStorage.
       * @param mutableStorage
       */
      virtual void commit(MutableStorage& mutableStorage) = 0;

      virtual ~Ametsuchi(){}
    };

  }  // namespace ametsuchi

}  // namespace iroha

#endif  // IROHA_AMETSUCHI_H

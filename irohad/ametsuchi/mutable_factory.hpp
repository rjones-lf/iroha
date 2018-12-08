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

#ifndef IROHA_MUTABLE_FACTORY_HPP
#define IROHA_MUTABLE_FACTORY_HPP

#include <memory>
#include "common/result.hpp"

namespace shared_model {
  namespace interface {
    class Block;
  }
}

namespace iroha {
  namespace ametsuchi {

    class MutableStorage;

    class MutableFactory {
     public:
      /**
       * Creates a mutable storage from the current state.
       * Mutable storage is the only way to commit the block to the ledger.
       * @return Created Result with mutable storage or error string
       */
      virtual expected::Result<std::unique_ptr<MutableStorage>, std::string>
      createMutableStorage() = 0;

      /**
       * Commit mutable storage to Ametsuchi.
       * This transforms Ametsuchi to the new state consistent with
       * MutableStorage.
       * @param mutableStorage
       */
      virtual void commit(std::unique_ptr<MutableStorage> mutableStorage) = 0;

      /**
       * Try to apply prepared block to Ametsuchi.
       * @return true if commit is succesful, false if prepared block failed
       * to apply. WSV is not changed if it returns false.
       *
       */
      virtual bool commitPrepared(const shared_model::interface::Block& block) = 0;

      virtual ~MutableFactory() = default;
    };

  }  // namespace ametsuchi
}  // namespace iroha
#endif  // IROHA_MUTABLE_FACTORY_HPP

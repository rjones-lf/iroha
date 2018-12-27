/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
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

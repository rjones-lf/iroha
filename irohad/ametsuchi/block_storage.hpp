/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_BLOCK_STORAGE_HPP
#define IROHA_BLOCK_STORAGE_HPP

#include <cstdint>
#include <functional>
#include <memory>

#include <boost/optional.hpp>
#include "interfaces/iroha_internal/block.hpp"

namespace iroha {
  namespace ametsuchi {

    /**
     * Append-only block storage interface
     */
    class BlockStorage {
     public:
      /**
       * Type of storage key
       */
      using Identifier = uint32_t;

      /**
       * Append block with given key
       * @return true if inserted successfully, false otherwise
       */
      virtual bool insert(
          Identifier id,
          std::shared_ptr<const shared_model::interface::Block> block) = 0;

      [[deprecated("Use shared_ptr")]] virtual bool insert(
          Identifier id, const shared_model::interface::Block &block) = 0;

      /**
       * Get block associated with given identifier
       * @return block if exists, boost::none otherwise
       */
      virtual boost::optional<
          std::shared_ptr<const shared_model::interface::Block>>
      fetch(Identifier id) const = 0;

      /**
       * Returns the size of the storage
       */
      virtual size_t size() const = 0;

      /**
       * Clears the contents of storage
       */
      virtual void clear() = 0;

      /// type of function which can be applied to the elements of the storage
      using FunctionType = std::function<void(
          Identifier, std::shared_ptr<const shared_model::interface::Block>)>;

      /**
       * Iterates through all the stored blocks
       */
      virtual void forEach(FunctionType function) const = 0;

      virtual ~BlockStorage() = default;
    };

  }  // namespace ametsuchi
}  // namespace iroha

#endif  // IROHA_BLOCK_STORAGE_HPP

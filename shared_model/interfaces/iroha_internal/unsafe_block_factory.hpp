/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_UNSAFE_BLOCK_FACTORY_HPP
#define IROHA_UNSAFE_BLOCK_FACTORY_HPP

#include "interfaces/iroha_internal/block_variant.hpp"

namespace shared_model {
  namespace interface {
    /**
     * UnsafeBlockFactory creates block without any validation
     */
    class UnsafeBlockFactory {
     public:
      /**
       * Create block
       * @param txs - if it is empty, create EmptyBlock,
       * else create regular block
       */
      virtual BlockVariant unsafeCreateBlock(
          types::HeightType height,
          const types::HashType &prev_hash,
          types::TimestampType created_time,
          types::TransactionsCollectionType txs) = 0;
    };
  }  // namespace interface
}  // namespace shared_model

#endif  // IROHA_UNSAFE_BLOCK_FACTORY_HPP

/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_BLOCK_FACTORY_HPP
#define IROHA_BLOCK_FACTORY_HPP

#include "common/result.hpp"
#include "interfaces/iroha_internal/block_variant.hpp"

namespace shared_model {
  namespace interface {
    /**
     * BlockFactory is an interface to create blocks
     */
    class BlockFactory {
     public:
      using BlockResult = iroha::expected::Result<BlockVariant, std::string>;

      /**
       * Create block
       * @param txs - if it is empty, create EmptyBlock,
       * else create regular block
       */
      virtual BlockResult createBlock(types::HeightType height,
                                      const types::HashType &prev_hash,
                                      types::TimestampType created_time,
                                      types::TransactionsCollectionType txs) = 0;
    };
  }  // namespace interface
}  // namespace shared_model
#endif  // IROHA_BLOCK_FACTORY_HPP

/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_CONSENSUS_BLOCK_CACHE_HPP
#define IROHA_CONSENSUS_BLOCK_CACHE_HPP

#include "cache/single_pointer_cache.hpp"
#include "interfaces/iroha_internal/block_variant.hpp"

namespace iroha {
  namespace consensus {

    /**
     * Type to represent result of the consensus in form of block/empty_block
     */
    using ConsensusResult = shared_model::interface::BlockVariant;

    /**
     * Type to represent a consensus result cache with a single block
     */
    using ConsensusResultCache =
        cache::SinglePointerCache<ConsensusResult>;

  }  // namespace consensus
}  // namespace iroha

#endif  // IROHA_CONSENSUS_BLOCK_CACHE_HPP

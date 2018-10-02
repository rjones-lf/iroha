/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_ROUND_HPP
#define IROHA_ROUND_HPP

#include <cstdint>

#include <boost/functional/hash.hpp>

namespace iroha {
  namespace consensus {

    /**
     * Type of round indexing by blocks
     */
    using BlockRoundType = uint64_t;

    /**
     * Type of round indexing by reject before new block commit
     */
    using RejectRoundType = uint32_t;

    /**
     * Type of proposal round
     */
    struct Round {
      BlockRoundType block_round;
      RejectRoundType reject_round;

      Round() = default;

      Round(BlockRoundType block_r, RejectRoundType reject_r);

      bool operator<(const Round &rhs) const;

      bool operator==(const Round &rhs) const;

      bool operator!=(const Round &rhs) const;
    };

    /**
     * Class provides hash function for Round
     */
    class RoundTypeHasher {
     public:
      std::size_t operator()(const consensus::Round &val) const {
        size_t seed = 0;
        boost::hash_combine(seed, val.block_round);
        boost::hash_combine(seed, val.reject_round);
        return seed;
      }
    };

  }  // namespace consensus
}  // namespace iroha

#endif  // IROHA_ROUND_HPP

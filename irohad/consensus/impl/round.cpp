/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "consensus/round.hpp"

#include <boost/functional/hash.hpp>
#include <tuple>
#include <utility>

namespace iroha {
  namespace consensus {
    Round::Round(BlockRoundType block_r, RejectRoundType reject_r)
        : block_round{block_r}, reject_round{reject_r} {}

    bool Round::operator<(const Round &rhs) const {
      return std::tie(block_round, reject_round)
          < std::tie(rhs.block_round, rhs.reject_round);
    }

    bool Round::operator==(const Round &rhs) const {
      return std::tie(block_round, reject_round)
          == std::tie(rhs.block_round, rhs.reject_round);
    }

    bool Round::operator!=(const Round &rhs) const {
      return not(*this == rhs);
    }

    std::size_t RoundTypeHasher::operator()(const consensus::Round &val) const {
      size_t seed = 0;
      boost::hash_combine(seed, val.block_round);
      boost::hash_combine(seed, val.reject_round);
      return seed;
    }
  }  // namespace consensus
}  // namespace iroha

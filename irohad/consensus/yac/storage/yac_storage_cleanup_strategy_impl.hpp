/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_YAC_STORAGE_CLEANUP_STRATEGY_IMPL_HPP
#define IROHA_YAC_STORAGE_CLEANUP_STRATEGY_IMPL_HPP

#include "consensus/yac/storage/yac_storage_cleanup_strategy.hpp"

#include <queue>

namespace iroha {
  namespace consensus {
    namespace yac {
      class BufferedCleanupStrategy : public CleanupStrategy {
       public:
        using QueueSizeType = size_t;
        using RoundType = Round;
        using QueueType = std::queue<Round>;

        BufferedCleanupStrategy(QueueSizeType max_limit,
                                RoundType minimal_round,
                                QueueType existed_state);

        boost::optional<CleanupStrategy::RoundsType> finalize(
            Round round, Answer answer) override;

        virtual bool shouldCreateRound(const Round &round) override;

       private:
        /// maximal size of queue
        const QueueSizeType max_size_;

        /// minimal round in queue
        RoundType min_round_;

        /// queue holds current rounds
        QueueType current_rounds_;
      };
    }  // namespace yac
  }    // namespace consensus
}  // namespace iroha

#endif  // IROHA_YAC_STORAGE_CLEANUP_STRATEGY_IMPL_HPP

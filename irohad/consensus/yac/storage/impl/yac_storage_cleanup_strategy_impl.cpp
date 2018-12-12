/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "consensus/yac/storage/yac_storage_cleanup_strategy_impl.hpp"

#include "consensus/yac/messages.hpp"

using namespace iroha::consensus::yac;

BufferedCleanupStrategy::BufferedCleanupStrategy(QueueSizeType max_limit,
                                                 RoundType minimal_round,
                                                 QueueType existed_state)
    : max_size_(max_limit),
      min_round_(std::move(minimal_round)),
      current_rounds_(std::move(existed_state)) {}

boost::optional<CleanupStrategy::RoundsType> BufferedCleanupStrategy::finalize(
    Round round, Answer answer) {
  CleanupStrategy::RoundsType outcome;
  if (current_rounds_.size() >= max_size_) {
    outcome.push_back(current_rounds_.front());
    current_rounds_.pop();
  }
  current_rounds_.push(round);

  if (min_round_ < round) {
    min_round_ = round;
  }

  if (outcome.empty()) {
    return boost::none;
  } else {
    return outcome;
  }
}

bool BufferedCleanupStrategy::shouldCreateRound(const Round &round) {
  return min_round_ < round;
}

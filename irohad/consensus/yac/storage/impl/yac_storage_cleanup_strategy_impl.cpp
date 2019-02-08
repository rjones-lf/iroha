/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "consensus/yac/storage/yac_storage_cleanup_strategy_impl.hpp"

using namespace iroha::consensus::yac;

BufferedCleanupStrategy::BufferedCleanupStrategy(QueueSizeType max_limit,
                                                 RoundType minimal_round,
                                                 QueueType existed_state)
    : kMaxSize_(max_limit),
      min_round_(std::move(minimal_round)),
      current_rounds_(std::move(existed_state)) {}

boost::optional<CleanupStrategy::RoundsType> BufferedCleanupStrategy::finalize(
    RoundType round, Answer answer) {
  CleanupStrategy::RoundsType outcome;
  if (current_rounds_.size() >= kMaxSize_) {
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
  // TODO: 13/12/2018 @muratovv possible DOS-attack on consensus IR-128
  return min_round_ <= round;
}

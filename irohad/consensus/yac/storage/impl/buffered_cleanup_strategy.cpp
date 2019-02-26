/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "consensus/yac/storage/buffered_cleanup_strategy.hpp"

#include "common/visitor.hpp"

using namespace iroha::consensus::yac;

boost::optional<CleanupStrategy::RoundsType> BufferedCleanupStrategy::finalize(
    RoundType consensus_round, Answer answer) {
  using OptRefRoundType = boost::optional<RoundType> &;
  auto &target_round = iroha::visit_in_place(
      answer,
      [this](
          const iroha::consensus::yac::CommitMessage &msg) -> OptRefRoundType {
        // greater commit removes last reject because previous rejects doesn't
        // required anymore for the consensus
        if (last_commit_round_ and last_reject_round_
            and *last_commit_round_ < *last_reject_round_) {
          last_reject_round_ = boost::none;
        }
        return last_commit_round_;
      },
      [this](const iroha::consensus::yac::RejectMessage &msg)
          -> OptRefRoundType { return last_reject_round_; });

  if (target_round) {
    if (*target_round < consensus_round) {
      target_round = consensus_round;
    }
  } else {
    target_round = consensus_round;
  }

  auto removed_rounds = truncateCreatedRounds();
  if (removed_rounds.empty()) {
    return boost::none;
  } else {
    return removed_rounds;
  }
}

CleanupStrategy::RoundsType BufferedCleanupStrategy::truncateCreatedRounds() {
  CleanupStrategy::RoundsType removed;
  if (last_commit_round_) {
    while (*last_commit_round_ > created_rounds_.top()) {
      removed.push_back(created_rounds_.top());
      created_rounds_.pop();
    }
  }
  return removed;
}

boost::optional<BufferedCleanupStrategy::RoundType>
BufferedCleanupStrategy::minimalRound() const {
  // both value unavailable
  if (not last_reject_round_ and not last_commit_round_) {
    return boost::none;
  }

  // both values present
  if (last_reject_round_ and last_commit_round_) {
    return *last_commit_round_ < *last_reject_round_ ? last_commit_round_
                                                     : last_reject_round_;
  }

  // one value presents
  if (last_commit_round_) {
    return last_commit_round_;
  } else {
    return last_reject_round_;
  }
}

bool BufferedCleanupStrategy::shouldCreateRound(const Round &round) {
  // TODO: 13/12/2018 @muratovv possible DOS-attack on consensus IR-128
  auto should_create = false;
  auto min_round = minimalRound();
  if (min_round) {
    should_create = min_round <= round;
  } else {
    should_create = true;
  }
  created_rounds_.push(round);
  return should_create;
}

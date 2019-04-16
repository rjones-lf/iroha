/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ordering/impl/kick_out_proposal_creation_strategy.hpp"

#include <algorithm>
#include "interfaces/common_objects/peer.hpp"

using namespace iroha::ordering;

KickOutProposalCreationStrategy::KickOutProposalCreationStrategy(
    std::shared_ptr<SupermajorityCheckerType> majority_checker)
    : majority_checker_(majority_checker) {}

void KickOutProposalCreationStrategy::onCollaborationOutcome(
    const PeerList &peers) {
  std::lock_guard<std::mutex> guard(mutex_);
  RoundCollectionType last_requested;
  for (const auto &peer : peers) {
    auto iter = last_requested_.find(peer.hex());
    if (iter != last_requested_.end()) {
      last_requested.insert(*iter);
    } else {
      last_requested.insert({peer.hex(), RoundType{0, 0}});
    }
  }
  last_requested_ = last_requested;
}

bool KickOutProposalCreationStrategy::shouldCreateRound(RoundType round) {
  uint64_t counter = 0;
  {
    std::lock_guard<std::mutex> guard(mutex_);
    counter = std::count_if(
        last_requested_.begin(),
        last_requested_.end(),
        [&round](const auto &elem) { return elem.second >= round; });
  }

  auto has_majority =
      majority_checker_->hasMajority(counter, last_requested_.size());
  return not has_majority;
}

boost::optional<ProposalCreationStrategy::RoundType>
KickOutProposalCreationStrategy::onProposal(const PeerType &who,
                                            RoundType requested_round) {
  {
    std::lock_guard<std::mutex> guard(mutex_);
    auto iter = last_requested_.find(who.hex());
    if (iter != last_requested_.end() and iter->second < requested_round) {
      iter->second = requested_round;
    }
  }

  return boost::none;
}

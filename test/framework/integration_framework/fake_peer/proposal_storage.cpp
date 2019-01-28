/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "framework/integration_framework/fake_peer/proposal_storage.hpp"

#include <mutex>

namespace integration_framework {
  namespace fake_peer {

    ProposalStorage::ProposalStorage()
        : default_provider_([](auto &) { return boost::none; }) {}

    OrderingProposalRequestResult ProposalStorage::getProposal(
        const Round &round) const {
      std::shared_lock<std::shared_timed_mutex> lock(proposals_map_mutex_);
      auto it = proposals_map_.find(round);
      if (it != proposals_map_.end()) {
        if (it->second) {
          return it->second;
        } else {
          return boost::none;
        }
      }
      return default_provider_(round);
    }

    ProposalStorage &ProposalStorage::storeProposal(
        const Round &round, std::shared_ptr<Proposal> proposal) {
      std::lock_guard<std::shared_timed_mutex> lock(proposals_map_mutex_);
      const auto it = proposals_map_.find(round);
      if (it == proposals_map_.end()) {
        proposals_map_.emplace(round, proposal);
      } else {
        it->second = proposal;
      }
      return *this;
    }

  }  // namespace fake_peer
}  // namespace integration_framework

/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "framework/integration_framework/fake_peer/proposal_storage.hpp"

namespace integration_framework {
  namespace fake_peer {

    ProposalStorage::ProposalStorage()
        : default_provider_([](auto &) { return boost::none; }) {}

    OrderingProposalRequestResult ProposalStorage::getProposal(
        const Round &round) const {
      auto it = proposals_map_.find(round);
      if (it != proposals_map_.end()) {
        if (it->second) {
          return *it->second;
        } else {
          return boost::none;
        }
      }
      return default_provider_(round);
    }

    ProposalStorage &ProposalStorage::storeProposal(
        const Round &round, std::shared_ptr<Proposal> proposal) {
      const auto it = proposals_map_.find(round);
      if (it == proposals_map_.end()) {
        proposals_map_.emplace(round, proposal);
      } else {
        proposals_map_.emplace_hint(proposals_map_.erase(it), round, proposal);
      }
      return *this;
    }

    ProposalStorage &ProposalStorage::setDefaultProvider(
        DefaultProvider default_provider) {
      default_provider_ = default_provider;
      return *this;
    }

  }  // namespace fake_peer
}  // namespace integration_framework

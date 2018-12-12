/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "consensus/yac/storage/yac_vote_storage.hpp"

#include <algorithm>
#include <utility>

#include "consensus/yac/consistency_model.hpp"
#include "consensus/yac/storage/yac_proposal_storage.hpp"

namespace iroha {
  namespace consensus {
    namespace yac {

      // --------| private api |--------

      auto YacVoteStorage::getProposalStorage(const Round &round) {
        return std::find_if(proposal_storages_.begin(),
                            proposal_storages_.end(),
                            [&round](const auto &storage) {
                              return storage.getStorageKey() == round;
                            });
      }

      auto YacVoteStorage::findProposalStorage(const VoteMessage &msg,
                                               PeersNumberType peers_in_round) {
        auto val = getProposalStorage(msg.hash.vote_round);
        if (val != proposal_storages_.end()) {
          return val;
        }
        return proposal_storages_.emplace(proposal_storages_.end(),
                                          msg.hash.vote_round,
                                          peers_in_round,
                                          supermajority_checker_);
      }

      // --------| public api |--------

      YacVoteStorage::YacVoteStorage(ConsistencyModel consistency_model)
          : supermajority_checker_(getSupermajorityChecker(consistency_model)) {
      }

      boost::optional<Answer> YacVoteStorage::store(
          std::vector<VoteMessage> state, PeersNumberType peers_in_round) {
        auto storage = findProposalStorage(state.at(0), peers_in_round);
        return storage->insert(state);
      }

      bool YacVoteStorage::isCommitted(const Round &round) {
        auto iter = getProposalStorage(round);
        if (iter == proposal_storages_.end()) {
          return false;
        }
        return bool(iter->getState());
      }

      ProposalState YacVoteStorage::getProcessingState(const Round &round) {
        return processing_state_[round];
      }

      void YacVoteStorage::nextProcessingState(const Round &round) {
        auto &val = processing_state_[round];
        switch (val) {
          case ProposalState::kNotSentNotProcessed:
            val = ProposalState::kSentNotProcessed;
            break;
          case ProposalState::kSentNotProcessed:
            val = ProposalState::kSentProcessed;
            break;
          case ProposalState::kSentProcessed:
            break;
        }
      }

    }  // namespace yac
  }    // namespace consensus
}  // namespace iroha

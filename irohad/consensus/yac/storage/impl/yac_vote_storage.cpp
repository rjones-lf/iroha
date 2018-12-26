/**
 * Copyright Soramitsu Co., Ltd. 2017 All Rights Reserved.
 * http://soramitsu.co.jp
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "consensus/yac/storage/yac_vote_storage.hpp"

#include <algorithm>
#include <utility>

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
        return proposal_storages_.emplace(
            proposal_storages_.end(),
            msg.hash.vote_round,
            peers_in_round,
            std::make_shared<SupermajorityCheckerImpl>());
      }

      // --------| public api |--------

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

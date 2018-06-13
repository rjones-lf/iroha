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

      auto YacVoteStorage::getProposalStorage(ProposalHash hash) {
        return std::find_if(proposal_storages_.begin(),
                            proposal_storages_.end(),
                            [&hash](auto storage) {
                              return storage.getProposalHash() == hash;
                            });
      }

      auto YacVoteStorage::findProposalStorage(const VoteMessage &msg,
                                               PeersNumberType peers_in_round) {
        auto val = getProposalStorage(msg.hash.proposal_hash);
        if (val != proposal_storages_.end()) {
          return val;
        }
        return proposal_storages_.emplace(
            proposal_storages_.end(),
            msg.hash.proposal_hash,
            peers_in_round,
            std::make_shared<SupermajorityCheckerImpl>());
      }

      // --------| public api |--------

      boost::optional<Answer> YacVoteStorage::store(
          std::vector<VoteMessage> state, PeersNumberType peers_in_round) {
        auto storage = findProposalStorage(state.at(0), peers_in_round);
        return storage->insert(state);
      }

      bool YacVoteStorage::isHashCommitted(ProposalHash hash) {
        auto iter = getProposalStorage(std::move(hash));
        if (iter == proposal_storages_.end()) {
          return false;
        }
        return bool(iter->getState());
      }

      ProposalState YacVoteStorage::getProcessingState(
          const ProposalHash &hash) {
        return processing_state_[hash];
      }

      void YacVoteStorage::nextProcessingState(const ProposalHash &hash) {
        auto &val = processing_state_[hash];
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

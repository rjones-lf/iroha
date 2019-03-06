/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "consensus/yac/storage/yac_vote_storage.hpp"

#include <algorithm>
#include <utility>

#include "common/bind.hpp"
#include "consensus/yac/consistency_model.hpp"
#include "consensus/yac/storage/yac_proposal_storage.hpp"
#include "logger/logger_manager.hpp"

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

      boost::optional<std::vector<YacProposalStorage>::iterator>
      YacVoteStorage::findProposalStorage(const VoteMessage &msg,
                                          PeersNumberType peers_in_round) {
        const auto &round = msg.hash.vote_round;
        auto val = getProposalStorage(round);
        if (val != proposal_storages_.end()) {
          return val;
        }
        if (strategy_->shouldCreateRound(round)) {
          return proposal_storages_.emplace(
              proposal_storages_.end(),
              msg.hash.vote_round,
              peers_in_round,
              supermajority_checker_,
              log_manager_->getChild("ProposalStorage"));
        } else {
          return boost::none;
        }
      }

      void YacVoteStorage::remove(const iroha::consensus::Round &round) {
        auto val = getProposalStorage(round);
        if (val != proposal_storages_.end()) {
          proposal_storages_.erase(val);
        }
        auto state = processing_state_.find(round);
        if (state != processing_state_.end()) {
          processing_state_.erase(state);
        }
      }

      // --------| public api |--------

      YacVoteStorage::YacVoteStorage(
          std::shared_ptr<CleanupStrategy> cleanup_strategy,
          std::unique_ptr<SupermajorityChecker> supermajority_checker,
          logger::LoggerManagerTreePtr log_manager)
          : strategy_(std::move(cleanup_strategy)),
            supermajority_checker_(std::move(supermajority_checker)),
            log_manager_(std::move(log_manager)) {}

      boost::optional<Answer> YacVoteStorage::store(
          std::vector<VoteMessage> state, PeersNumberType peers_in_round) {
        if (state.empty()) {
          return boost::none;
        }
        return findProposalStorage(state.at(0), peers_in_round) |
            [this, &state](auto &&storage) {
              const auto &round = storage->getStorageKey();
              return storage->insert(state) |
                         [this, &round](
                             auto &&insert_outcome) -> boost::optional<Answer> {
                this->strategy_->finalize(round, insert_outcome) |
                    [this](auto &&remove) {
                      std::for_each(
                          remove.begin(),
                          remove.end(),
                          [this](const auto &round) { this->remove(round); });
                    };
                return insert_outcome;
              };
            };
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

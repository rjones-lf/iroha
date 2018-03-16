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

#include "consensus/yac/storage/yac_proposal_storage.hpp"

using namespace logger;

namespace iroha {
  namespace consensus {
    namespace yac {

      // --------| private api |--------

      auto YacProposalStorage::findStore(ProposalHash proposal_hash,
                                         BlockHash block_hash) {
        // find exist
        auto iter =
            std::find_if(block_storages_.begin(),
                         block_storages_.end(),
                         [&proposal_hash, &block_hash](auto block_storage) {
                           auto yac_hash = block_storage.getStorageHash();
                           return yac_hash.proposal_hash == proposal_hash
                               and yac_hash.block_hash == block_hash;
                         });
        if (iter != block_storages_.end()) {
          return iter;
        }
        // insert and return new
        return block_storages_.emplace(
            block_storages_.end(),
            YacHash(proposal_hash, block_hash),
            peers_in_round_,
            supermajority_checker_);
      }

      // --------| public api |--------

      YacProposalStorage::YacProposalStorage(
          ProposalHash hash,
          uint64_t peers_in_round,
          std::shared_ptr<SupermajorityChecker> supermajority_checker)
          : current_state_(boost::none),
            hash_(std::move(hash)),
            peers_in_round_(peers_in_round),
            supermajority_checker_(supermajority_checker) {
        log_ = log("ProposalStorage");
      }

      boost::optional<Answer> YacProposalStorage::insert(VoteMessage msg) {
        if (shouldInsert(msg)) {
          // insert to block store

          log_->info("Vote [{}, {}] looks valid",
                     msg.hash.proposal_hash,
                     msg.hash.block_hash);

          auto iter = findStore(msg.hash.proposal_hash, msg.hash.block_hash);
          auto block_state = iter->insert(msg);

          // Single BlockStorage always returns CommitMessage because it
          // aggregates votes for a single hash.
          if (block_state) {
            // supermajority on block achieved
            current_state_ = std::move(block_state);
          } else {
            // try to find reject case
            auto reject_state = findRejectProof();
            if (reject_state) {
              log_->info("Found reject proof");
              current_state_ = std::move(reject_state);
            }
          }
        }
        return getState();
      }

      boost::optional<Answer> YacProposalStorage::insert(
          std::vector<VoteMessage> messages) {
        std::for_each(messages.begin(), messages.end(), [this](auto vote) {
          this->insert(std::move(vote));
        });
        return getState();
      }
      ProposalHash YacProposalStorage::getProposalHash() {
        return hash_;
      }

      boost::optional<Answer> YacProposalStorage::getState() const {
        return current_state_;
      }

      // --------| private api |--------

      bool YacProposalStorage::shouldInsert(const VoteMessage &msg) {
        return checkProposalHash(msg.hash.proposal_hash)
            and checkPeerUniqueness(msg);
      }

      bool YacProposalStorage::checkProposalHash(ProposalHash vote_hash) {
        return vote_hash == hash_;
      }

      bool YacProposalStorage::checkPeerUniqueness(const VoteMessage &msg) {
        return std::all_of(block_storages_.begin(),
                           block_storages_.end(),
                           [&msg](YacBlockStorage &storage) {
                             if (storage.getStorageHash() != msg.hash) {
                               return true;
                             }
                             return not storage.isContains(msg);
                           });
      }

      boost::optional<Answer> YacProposalStorage::findRejectProof() {
        auto max_vote = std::max_element(block_storages_.begin(),
                                         block_storages_.end(),
                                         [](auto &left, auto &right) {
                                           return left.getNumberOfVotes()
                                               < right.getNumberOfVotes();
                                         })
                            ->getNumberOfVotes();

        auto all_votes =
            std::accumulate(block_storages_.begin(),
                            block_storages_.end(),
                            0ull,
                            [](auto &acc, auto &storage) {
                              return acc + storage.getNumberOfVotes();
                            });

        auto is_reject = supermajority_checker_->hasReject(
            max_vote, all_votes, peers_in_round_);

        if (is_reject) {
          std::vector<VoteMessage> result;
          result.reserve(all_votes);
          std::for_each(block_storages_.begin(),
                        block_storages_.end(),
                        [&result](auto &storage) {
                          auto votes_from_block_storage = storage.getVotes();
                          std::move(votes_from_block_storage.begin(),
                                    votes_from_block_storage.end(),
                                    std::back_inserter(result));
                        });

          return Answer(RejectMessage(std::move(result)));
        }

        return boost::none;
      }

    }  // namespace yac
  }    // namespace consensus
}  // namespace iroha

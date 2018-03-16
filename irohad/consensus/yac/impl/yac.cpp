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

#include <utility>

#include "common/visitor.hpp"
#include "consensus/yac/cluster_order.hpp"
#include "consensus/yac/storage/yac_proposal_storage.hpp"
#include "consensus/yac/timer.hpp"
#include "consensus/yac/yac.hpp"
#include "consensus/yac/yac_crypto_provider.hpp"

namespace iroha {
  namespace consensus {
    namespace yac {

      template <typename T>
      static std::string cryptoError(const T &votes) {
        std::string result =
            "Crypto verification failed for message.\n Votes: ";
        result += logger::to_string(votes, [](const auto &vote) {
          std::string result = "(Public key: ";
          result += vote.signature.pubkey.to_hexstring();
          result += ", Signature: ";
          result += vote.signature.signature.to_hexstring();
          result += ")\n";
          return result;
        });
        return result;
      }

      template <typename T>
      static std::string cryptoError(const std::initializer_list<T> &votes) {
        return cryptoError<std::initializer_list<T>>(votes);
      }

      std::shared_ptr<Yac> Yac::create(
          YacVoteStorage vote_storage,
          std::shared_ptr<YacNetwork> network,
          std::shared_ptr<YacCryptoProvider> crypto,
          std::shared_ptr<Timer> timer,
          ClusterOrdering order,
          uint64_t delay) {
        return std::make_shared<Yac>(
            vote_storage, network, crypto, timer, order, delay);
      }

      Yac::Yac(YacVoteStorage vote_storage,
               std::shared_ptr<YacNetwork> network,
               std::shared_ptr<YacCryptoProvider> crypto,
               std::shared_ptr<Timer> timer,
               ClusterOrdering order,
               uint64_t delay)
          : vote_storage_(std::move(vote_storage)),
            network_(std::move(network)),
            crypto_(std::move(crypto)),
            timer_(std::move(timer)),
            cluster_order_(order),
            delay_(delay) {
        log_ = logger::log("YAC");
      }

      // ------|Hash gate|------

      void Yac::vote(YacHash hash, ClusterOrdering order) {
        log_->info("Order for voting: {}",
                   logger::to_string(order.getPeers(),
                                     [](auto val) { return val.address; }));

        cluster_order_ = order;
        auto vote = crypto_->getVote(hash);
        votingStep(vote);
      }

      rxcpp::observable<CommitMessage> Yac::on_commit() {
        return notifier_.get_observable();
      }

      // ------|Network notifications|------

      void Yac::on_vote(VoteMessage vote) {
        std::lock_guard<std::mutex> guard(mutex_);
        if (crypto_->verify(vote)) {
          applyVote(findPeer(vote), vote);
        } else {
          log_->warn(cryptoError({vote}));
        }
      }

      void Yac::on_commit(CommitMessage commit) {
        std::lock_guard<std::mutex> guard(mutex_);
        if (crypto_->verify(commit)) {
          // Commit does not contain data about peer which sent the message
          applyCommit(boost::none, commit);
        } else {
          log_->warn(cryptoError(commit.votes));
        }
      }

      void Yac::on_reject(RejectMessage reject) {
        std::lock_guard<std::mutex> guard(mutex_);
        if (crypto_->verify(reject)) {
          // Reject does not contain data about peer which sent the message
          applyReject(boost::none, reject);
        } else {
          log_->warn(cryptoError(reject.votes));
        }
      }

      // ------|Private interface|------

      void Yac::votingStep(VoteMessage vote) {
        auto committed = vote_storage_.isHashCommitted(vote.hash.proposal_hash);
        if (committed) {
          return;
        }

        log_->info("Vote for hash ({}, {})",
                   vote.hash.proposal_hash,
                   vote.hash.block_hash);

        network_->send_vote(cluster_order_.currentLeader(), vote);
        cluster_order_.switchToNext();
        if (cluster_order_.hasNext()) {
          timer_->invokeAfterDelay(delay_,
                                   [this, vote] { this->votingStep(vote); });
        }
      }

      void Yac::closeRound() {
        timer_->deny();
      }

      boost::optional<model::Peer> Yac::findPeer(const VoteMessage &vote) {
        auto peers = cluster_order_.getPeers();
        auto it =
            std::find_if(peers.begin(), peers.end(), [&](const auto &peer) {
              return peer.pubkey == vote.signature.pubkey;
            });
        return it != peers.end() ? boost::make_optional(std::move(*it)) : boost::none;
      }

      // ------|Apply data|------

      const char *kRejectMsg = "reject case";
      const char *kRejectOnHashMsg = "Reject case on hash {} achieved";

      void Yac::applyCommit(boost::optional<model::Peer> from,
                            CommitMessage commit) {
        auto answer =
            vote_storage_.store(commit, cluster_order_.getNumberOfPeers());
        answer | [&](const auto &answer) {
          auto proposal_hash = getProposalHash(commit.votes).value();
          auto already_processed =
              vote_storage_.getProcessingState(proposal_hash);
          if (not already_processed) {
            vote_storage_.markAsProcessedState(proposal_hash);
            visit_in_place(answer,
                           [&](const CommitMessage &commit) {
                             notifier_.get_subscriber().on_next(commit);
                           },
                           [&](const RejectMessage &reject) {
                             log_->warn(kRejectMsg);
                             // TODO 14/08/17 Muratov: work on reject case
                             // IR-497
                           });
          }
          this->closeRound();
        };
      }

      void Yac::applyReject(boost::optional<model::Peer> from,
                            RejectMessage reject) {
        auto answer =
            vote_storage_.store(reject, cluster_order_.getNumberOfPeers());
        answer | [&](const auto &answer) {
          auto proposal_hash = getProposalHash(reject.votes).value();
          auto already_processed =
              vote_storage_.getProcessingState(proposal_hash);

          if (not already_processed) {
            vote_storage_.markAsProcessedState(proposal_hash);
            visit_in_place(answer,
                           [&](const RejectMessage &reject) {
                             log_->warn(kRejectMsg);
                             // TODO 14/08/17 Muratov: work on reject case
                             // IR-497
                           },
                           [&](const CommitMessage &commit) {
                             notifier_.get_subscriber().on_next(commit);
                             this->propagateCommit(commit);
                           });
          }
          this->closeRound();
        };
      }

      void Yac::applyVote(boost::optional<model::Peer> from,
                          VoteMessage vote) {
        if (from) {
          log_->info("Apply vote: {} from ledger peer {}",
                     vote.hash.block_hash,
                     (*from).address);
        } else {
          log_->info("Apply vote: {} from unknown peer {}",
                     vote.hash.block_hash,
                     vote.signature.pubkey.to_hexstring());
        }

        auto answer =
            vote_storage_.store(vote, cluster_order_.getNumberOfPeers());

        answer | [&](const auto &answer) {
          auto &proposal_hash = vote.hash.proposal_hash;
          auto already_processed =
              vote_storage_.getProcessingState(proposal_hash);

          if (not already_processed) {
            vote_storage_.markAsProcessedState(proposal_hash);
            visit_in_place(answer,
                           [&](const CommitMessage &commit) {
                             // propagate for all
                             log_->info("Propagate commit {} to whole network",
                                        vote.hash.block_hash);
                             notifier_.get_subscriber().on_next(commit);
                             this->propagateCommit(commit);
                           },
                           [&](const RejectMessage &reject) {
                             // propagate reject for all
                             log_->info(kRejectOnHashMsg, proposal_hash);
                             this->propagateReject(reject);
                           });
          } else {
            from | [&](const auto &from) {
              visit_in_place(answer,
                             [&](const CommitMessage &commit) {
                               log_->info("Propagate commit {} directly to {}",
                                          vote.hash.block_hash,
                                          from.address);
                               this->propagateCommitDirectly(from, commit);
                             },
                             [&](const RejectMessage &reject) {
                               log_->info(kRejectOnHashMsg, proposal_hash);
                               this->propagateRejectDirectly(from, reject);
                             });
            };
          }
        };
      }

      // ------|Propagation|------

      void Yac::propagateCommit(CommitMessage msg) {
        for (const auto &peer : cluster_order_.getPeers()) {
          propagateCommitDirectly(peer, msg);
        }
      }

      void Yac::propagateCommitDirectly(model::Peer to, CommitMessage msg) {
        network_->send_commit(std::move(to), std::move(msg));
      }

      void Yac::propagateReject(RejectMessage msg) {
        for (const auto &peer : cluster_order_.getPeers()) {
          propagateRejectDirectly(peer, msg);
        }
      }

      void Yac::propagateRejectDirectly(model::Peer to, RejectMessage msg) {
        network_->send_reject(std::move(to), std::move(msg));
      }

    }  // namespace yac
  }    // namespace consensus
}  // namespace iroha

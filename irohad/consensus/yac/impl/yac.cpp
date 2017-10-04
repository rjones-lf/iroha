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

#include "consensus/yac/yac.hpp"

namespace iroha {
  namespace consensus {
    namespace yac {

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

        this->cluster_order_ = order;
        auto vote = crypto_->getVote(hash);
        votingStep(vote);
      };

      rxcpp::observable<CommitMessage> Yac::on_commit() {
        return this->notifier_.get_observable();
      }

      // ------|Network notifications|------

      void Yac::on_vote(model::Peer from, VoteMessage vote) {
        std::lock_guard<std::mutex> guard(mutex_);
        if (not crypto_->verify(vote)) {
          log_->warn(
              "Crypto verification failed for vote from {}. Signature: {}, "
              "public key: {}",
              from.address,
              vote.signature.signature.to_hexstring(),
              vote.signature.pubkey.to_hexstring());
        } else {
          this->applyVote(from, vote);
        }
      }

      void Yac::on_commit(model::Peer from, CommitMessage commit) {
        std::lock_guard<std::mutex> guard(mutex_);
        if (crypto_->verify(commit)) {
          this->applyCommit(from, commit);
        } else {
          log_->warn("Crypto verification failed for commit from {}",
                     from.address);
          for (const auto &vote : commit.votes) {
            log_->warn("Signature: {}, public key: {}",
                       vote.signature.signature.to_hexstring(),
                       vote.signature.pubkey.to_hexstring());
          }
        }
      }

      void Yac::on_reject(model::Peer from, RejectMessage reject) {
        std::lock_guard<std::mutex> guard(mutex_);
        if (crypto_->verify(reject)) {
          this->applyReject(from, reject);
        } else {
          log_->warn("Crypto verification failed for vote from {}",
                     from.address);
          for (const auto &vote : reject.votes) {
            log_->warn("Signature: {}, public key: {}",
                       vote.signature.signature.to_hexstring(),
                       vote.signature.pubkey.to_hexstring());
          }
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

      void Yac::closeRound() { timer_->deny(); };

      // ------|Apply data|------

      void Yac::applyCommit(model::Peer from, CommitMessage commit) {
        auto answer =
            vote_storage_.store(commit, cluster_order_.getNumberOfPeers());
        if (not answer.has_value()) {
          // commit don't applied
          return;
        }

        auto proposal_hash = getProposalHash(commit.votes).value();
        auto already_processed =
            vote_storage_.getProcessingState(proposal_hash);

        if (not already_processed) {
          if (answer->commit.has_value()) {
            notifier_.get_subscriber().on_next(*answer->commit);
          }

          if (answer->reject.has_value()) {
            log_->warn("reject case");
            // todo work on reject case
          }
          vote_storage_.markAsProcessedState(proposal_hash);
        }
        closeRound();
      };

      void Yac::applyReject(model::Peer from, RejectMessage reject) {
        // todo apply to vote storage
        closeRound();
      };

      void Yac::applyVote(model::Peer from, VoteMessage vote) {
        log_->info(
            "Apply vote: {} from {}", vote.hash.block_hash, from.address);

        auto answer =
            vote_storage_.store(vote, cluster_order_.getNumberOfPeers());

        if (not answer.has_value()) {
          // commit don't applied
          return;
        }

        auto proposal_hash = vote.hash.proposal_hash;
        auto already_processed =
            vote_storage_.getProcessingState(proposal_hash);

        if (answer->commit.has_value()) {
          if (not already_processed) {
            // propagate for all

            log_->info("Propagate commit {} to whole network",
                       vote.hash.block_hash);

            propagateCommit(answer->commit.value());
          } else {
            // propagate directly

            log_->info("Propagate commit {} directly to {}",
                       vote.hash.block_hash,
                       from.address);

            propagateCommitDirectly(from, answer->commit.value());
          }
        }

        if (answer->reject.has_value()) {
          log_->info("Reject case on hash {} achieved", proposal_hash);

          if (not already_processed) {
            // propagate reject for all
            propagateReject(answer->reject.value());
          } else {
            // propagate directly
            propagateRejectDirectly(from, answer->reject.value());
          }
        }
      };

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

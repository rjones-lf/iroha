/**
 * Copyright Soramitsu Co., Ltd. 2018 All Rights Reserved.
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

#include "consensus/yac/yac.hpp"

#include <utility>

#include "common/types.hpp"
#include "common/visitor.hpp"
#include "consensus/yac/cluster_order.hpp"
#include "consensus/yac/storage/yac_proposal_storage.hpp"
#include "consensus/yac/timer.hpp"
#include "consensus/yac/yac_crypto_provider.hpp"
#include "cryptography/public_key.hpp"
#include "cryptography/signed.hpp"
#include "interfaces/common_objects/peer.hpp"

namespace iroha {
  namespace consensus {
    namespace yac {

      template <typename T>
      static std::string cryptoError(const T &votes) {
        std::string result =
            "Crypto verification failed for message.\n Votes: ";
        result += logger::to_string(votes, [](const auto &vote) {
          std::string result = "(Public key: ";
          result += vote.signature->publicKey().hex();
          result += ", Signature: ";
          result += vote.signature->signedData().hex();
          result += ")\n";
          return result;
        });
        return result;
      }

      std::shared_ptr<Yac> Yac::create(
          YacVoteStorage vote_storage,
          std::shared_ptr<YacNetwork> network,
          std::shared_ptr<YacCryptoProvider> crypto,
          std::shared_ptr<Timer> timer,
          ClusterOrdering order) {
        return std::make_shared<Yac>(
            vote_storage, network, crypto, timer, order);
      }

      Yac::Yac(YacVoteStorage vote_storage,
               std::shared_ptr<YacNetwork> network,
               std::shared_ptr<YacCryptoProvider> crypto,
               std::shared_ptr<Timer> timer,
               ClusterOrdering order)
          : vote_storage_(std::move(vote_storage)),
            network_(std::move(network)),
            crypto_(std::move(crypto)),
            timer_(std::move(timer)),
            cluster_order_(order) {
        log_ = logger::log("YAC");
      }

      // ------|Hash gate|------

      void Yac::vote(YacHash hash, ClusterOrdering order) {
        log_->info("Order for voting: {}",
                   logger::to_string(order.getPeers(),
                                     [](auto val) { return val->address(); }));

        cluster_order_ = order;
        auto vote = crypto_->getVote(hash);
        // TODO 10.06.2018 andrei: IR-1407 move YAC propagation strategy to a
        // separate entity
        votingStep(vote);
      }

      rxcpp::observable<Answer> Yac::onOutcome() {
        return notifier_.get_observable();
      }

      // ------|Network notifications|------

      void Yac::onState(std::vector<VoteMessage> state) {
        std::lock_guard<std::mutex> guard(mutex_);
        if (crypto_->verify(state)) {
          applyState(state);
        } else {
          log_->warn(cryptoError(state));
        }
      }

      // ------|Private interface|------

      void Yac::votingStep(VoteMessage vote) {
        auto committed = vote_storage_.isCommitted(vote.hash.vote_round);
        if (committed) {
          return;
        }

        log_->info("Vote for round ({}, {}), hash ({}, {})",
                   vote.hash.vote_round.block_round,
                   vote.hash.vote_round.reject_round,
                   vote.hash.vote_hashes.proposal_hash,
                   vote.hash.vote_hashes.block_hash);

        network_->sendState(cluster_order_.currentLeader(), {vote});
        cluster_order_.switchToNext();
        if (cluster_order_.hasNext()) {
          timer_->invokeAfterDelay([this, vote] { this->votingStep(vote); });
        }
      }

      void Yac::closeRound() {
        timer_->deny();
      }

      boost::optional<std::shared_ptr<shared_model::interface::Peer>>
      Yac::findPeer(const VoteMessage &vote) {
        auto peers = cluster_order_.getPeers();
        auto it =
            std::find_if(peers.begin(), peers.end(), [&](const auto &peer) {
              return peer->pubkey() == vote.signature->publicKey();
            });
        return it != peers.end() ? boost::make_optional(std::move(*it))
                                 : boost::none;
      }

      // ------|Apply data|------

      void Yac::applyState(const std::vector<VoteMessage> &state) {
        auto answer =
            vote_storage_.store(state, cluster_order_.getNumberOfPeers());

        // TODO 10.06.2018 andrei: IR-1407 move YAC propagation strategy to a
        // separate entity

        answer | [&](const auto &answer) {
          auto &proposal_round = state.at(0).hash.vote_round;

          /*
           * It is possible that a new peer with an outdated peers list may
           * collect an outcome from a smaller number of peers which are
           * included in set of `f` peers in the system. The new peer will not
           * accept our message with valid supermajority because he cannot apply
           * votes from unknown peers.
           */
          if (state.size() > 1) {
            // some peer has already collected commit/reject, so it is sent
            if (vote_storage_.getProcessingState(proposal_round)
                == ProposalState::kNotSentNotProcessed) {
              vote_storage_.nextProcessingState(proposal_round);
              log_->info(
                  "Received supermajority of votes for ({}, {}), skip "
                  "propagation",
                  proposal_round.block_round,
                  proposal_round.reject_round);
            }
          }

          auto processing_state =
              vote_storage_.getProcessingState(proposal_round);

          auto votes = [](const auto &state) { return state.votes; };

          switch (processing_state) {
            case ProposalState::kNotSentNotProcessed:
              vote_storage_.nextProcessingState(proposal_round);
              log_->info("Propagate state ({}, {}) to whole network",
                         proposal_round.block_round,
                         proposal_round.reject_round);
              this->propagateState(visit_in_place(answer, votes));
              break;
            case ProposalState::kSentNotProcessed:
              vote_storage_.nextProcessingState(proposal_round);
              log_->info("Pass outcome for ({}, {}) to pipeline",
                         proposal_round.block_round,
                         proposal_round.reject_round);
              this->closeRound();
              notifier_.get_subscriber().on_next(answer);
              break;
            case ProposalState::kSentProcessed:
              if (state.size() == 1) {
                this->findPeer(state.at(0)) | [&](const auto &from) {
                  log_->info("Propagate state ({}, {}) directly to {}",
                             proposal_round.block_round,
                             proposal_round.reject_round,
                             from->address());
                  this->propagateStateDirectly(*from,
                                               visit_in_place(answer, votes));
                };
              }
              break;
          }
        };
      }

      // ------|Propagation|------

      void Yac::propagateState(const std::vector<VoteMessage> &msg) {
        for (const auto &peer : cluster_order_.getPeers()) {
          propagateStateDirectly(*peer, msg);
        }
      }

      void Yac::propagateStateDirectly(const shared_model::interface::Peer &to,
                                       const std::vector<VoteMessage> &msg) {
        network_->sendState(to, msg);
      }

    }  // namespace yac
  }    // namespace consensus
}  // namespace iroha

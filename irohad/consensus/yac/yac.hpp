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

#ifndef IROHA_YAC_HPP
#define IROHA_YAC_HPP

#include <memory>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <mutex>
#include <nonstd/optional.hpp>

#include "consensus/yac/yac_gate.hpp"
#include "consensus/yac/yac_network_interface.hpp"
#include "consensus/yac/yac_crypto_provider.hpp"
#include "consensus/yac/timer.hpp"
#include "consensus/yac/storage/yac_vote_storage.hpp"
#include "logger/logger.hpp"

namespace iroha {
  namespace consensus {
    namespace yac {
      class Yac : public HashGate,
                  public YacNetworkNotifications {
       public:
        /**
         * Method for creating Yac consensus object
         * @param delay for timer in milliseconds
         */
        static std::shared_ptr<Yac> create(
            YacVoteStorage vote_storage,
            std::shared_ptr<YacNetwork> network,
            std::shared_ptr<YacCryptoProvider> crypto,
            std::shared_ptr<Timer> timer,
            ClusterOrdering order,
            uint64_t delay);

        Yac(YacVoteStorage vote_storage,
            std::shared_ptr<YacNetwork> network,
            std::shared_ptr<YacCryptoProvider> crypto,
            std::shared_ptr<Timer> timer,
            ClusterOrdering order,
            uint64_t delay);

        // ------|Hash gate|------

        virtual void vote(YacHash hash, ClusterOrdering order);

        virtual rxcpp::observable<CommitMessage> on_commit();

        // ------|Network notifications|------

        virtual void on_commit(model::Peer from, CommitMessage commit);

        virtual void on_reject(model::Peer from, RejectMessage reject);

        virtual void on_vote(model::Peer from, VoteMessage vote);

       private:
        // ------|Private interface|------

        /**
         * Voting step is strategy of propagating vote
         * until commit/reject message received
         */
        void votingStep(YacHash hash);

        /**
         * Erase temporary data of current round
         */
        void closeRound();

        // ------|Apply data|------
        void applyCommit(model::Peer from, CommitMessage commit);
        void applyReject(model::Peer from, RejectMessage reject);
        void applyVote(model::Peer from, VoteMessage vote);

        // ------|Propagation|------
        void propagateCommit(CommitMessage msg);
        void propagateCommitDirectly(model::Peer to, CommitMessage msg);
        void propagateReject(RejectMessage msg);
        void propagateRejectDirectly(model::Peer to, RejectMessage msg);

        // ------|Fields|------
        YacVoteStorage vote_storage_;
        std::shared_ptr<YacNetwork> network_;
        std::shared_ptr<YacCryptoProvider> crypto_;
        std::shared_ptr<Timer> timer_;
        rxcpp::subjects::subject<CommitMessage> notifier_;
        std::mutex mutex_;

        // ------|One round|------
        ClusterOrdering cluster_order_;

        // ------|Constants|------
        const uint64_t delay_;

        // ------|Logger|------
        logger::Logger log_;

      };
    }  // namespace yac
  }    // namespace consensus
}  // namespace iroha

#endif  // IROHA_YAC_HPP

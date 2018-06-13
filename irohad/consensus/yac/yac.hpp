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

#include <boost/optional.hpp>
#include <memory>
#include <mutex>
#include <rxcpp/rx-observable.hpp>

#include "consensus/yac/cluster_order.hpp"  //  for ClusterOrdering
#include "consensus/yac/messages.hpp"       // because messages passed by value
#include "consensus/yac/storage/yac_vote_storage.hpp"  // for VoteStorage
#include "consensus/yac/transport/yac_network_interface.hpp"  // for YacNetworkNotifications
#include "consensus/yac/yac_gate.hpp"                         // for HashGate
#include "logger/logger.hpp"

namespace iroha {
  namespace consensus {
    namespace yac {

      class YacCryptoProvider;
      class Timer;

      class Yac : public HashGate, public YacNetworkNotifications {
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
            ClusterOrdering order);

        Yac(YacVoteStorage vote_storage,
            std::shared_ptr<YacNetwork> network,
            std::shared_ptr<YacCryptoProvider> crypto,
            std::shared_ptr<Timer> timer,
            ClusterOrdering order);

        // ------|Hash gate|------

        void vote(YacHash hash, ClusterOrdering order) override;

        rxcpp::observable<Answer> onOutcome() override;

        // ------|Network notifications|------

        void onState(std::vector<VoteMessage> state) override;

       private:
        // ------|Private interface|------

        /**
         * Voting step is strategy of propagating vote
         * until commit/reject message received
         */
        void votingStep(VoteMessage vote);

        /**
         * Erase temporary data of current round
         */
        void closeRound();

        /**
         * Find corresponding peer in the ledger from vote message
         * @param vote message containing peer information
         * @return peer if it is present in the ledger, boost::none otherwise
         */
        boost::optional<std::shared_ptr<shared_model::interface::Peer>>
        findPeer(const VoteMessage &vote);

        // ------|Apply data|------
        void applyState(const std::vector<VoteMessage> &state);

        // ------|Propagation|------
        void propagateState(const std::vector<VoteMessage> &msg);
        void propagateStateDirectly(const shared_model::interface::Peer &to,
                                    const std::vector<VoteMessage> &msg);

        // ------|Fields|------
        YacVoteStorage vote_storage_;
        std::shared_ptr<YacNetwork> network_;
        std::shared_ptr<YacCryptoProvider> crypto_;
        std::shared_ptr<Timer> timer_;
        rxcpp::subjects::subject<Answer> notifier_;
        std::mutex mutex_;

        // ------|One round|------
        ClusterOrdering cluster_order_;

        // ------|Logger|------
        logger::Logger log_;
      };
    }  // namespace yac
  }    // namespace consensus
}  // namespace iroha

#endif  // IROHA_YAC_HPP

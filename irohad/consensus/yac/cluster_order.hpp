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

#ifndef IROHA_CLUSTER_ORDER_HPP
#define IROHA_CLUSTER_ORDER_HPP

#include <vector>
#include "model/peer.hpp"

namespace iroha {
  namespace consensus {
    namespace yac {

      /**
       * Provider for the validator order for the current consensus round.
       *
       * TODO: I don't like the name Cluster. ValidatorOrdering is better.
       * -ing is also unnecessary, so just ValidatorOrder is fine.
       */
      class ClusterOrdering {
       public:
        /**
         * Creates the validator ordering from the vector of peers.
         *
         * @param order vector of validating peers (validators)
         * @return false if vector is empty, true otherwise
         */
        static nonstd::optional<ClusterOrdering> create(
            const std::vector<model::Peer> &order);

        /**
         * Provide current leader peer
         */
        model::Peer currentLeader();

        /**
         * Switch to next peer as leader
         * @return this
         */
        ClusterOrdering &switchToNext();

        /**
         * @return true if current leader not last peer in order
         */
        bool hasNext();

        std::vector<model::Peer> getPeers() {
          return order_;
        };

        auto getNumberOfPeers() {
          return order_.size();
        }

        virtual ~ClusterOrdering() = default;

       private:
        // prohibit creation of the object not from create method
        explicit ClusterOrdering(std::vector<model::Peer> order);
        ClusterOrdering() = delete;

        std::vector<model::Peer> order_;
        uint32_t index_ = 0;
      };
    }  // namespace yac
  }    // namespace consensus
}  // namespace iroha
#endif  // IROHA_CLUSTER_ORDER_HPP

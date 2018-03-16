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

#include <boost/optional.hpp>
#include <vector>
#include "model/peer.hpp"  // for Peer, because currentLeader() returns by value

namespace iroha {
  namespace consensus {
    namespace yac {

      /**
       * Class provide ordering on cluster for current round
       */
      class ClusterOrdering {
       public:
        /**
         * Creates cluster ordering from the vector of peers
         * @param order vector of peers
         * @return false if vector is empty, true otherwise
         */
        static boost::optional<ClusterOrdering> create(
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
        bool hasNext() const;

        std::vector<model::Peer> getPeers() const;

        size_t getNumberOfPeers() const;

        virtual ~ClusterOrdering() = default;

        ClusterOrdering() = delete;

       private:
        // prohibit creation of the object not from create method
        explicit ClusterOrdering(std::vector<model::Peer> order);

        std::vector<model::Peer> order_;
        uint32_t index_ = 0;
      };
    }  // namespace yac
  }    // namespace consensus
}  // namespace iroha
#endif  // IROHA_CLUSTER_ORDER_HPP

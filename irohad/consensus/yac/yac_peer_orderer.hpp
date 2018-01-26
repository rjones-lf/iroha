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

#ifndef IROHA_YAC_PEER_ORDERER_HPP
#define IROHA_YAC_PEER_ORDERER_HPP

#include <nonstd/optional.hpp>
#include "consensus/yac/cluster_order.hpp"
#include "consensus/yac/yac_hash_provider.hpp"

//TODO: Please review as to whether it is really necessary to have both a YacPeerOrderer and a ClusterOrdering.
//It seems nothing is gained in terms of encapsulation, as the validator ordering is the heart and soul of YAC and
//is not something that is going to be pluggable. The negative effect of splitting this up is you have more files
//for people to spend time going through and it hurts understandability.
namespace iroha {
  namespace consensus {
    namespace yac {

      /**
       * Interface responsible for creating order for yac consensus
       */
      class YacPeerOrderer {
       public:
        /**
         * Provide initial order for voting; useful when consensus has been initialized,
         * but not voted before.
         *
         * @return ordering, like in ledger
         */
        virtual nonstd::optional<ClusterOrdering> getInitialOrdering() = 0;

        /**
         * Provide order of peers based on hash and initial order of peers
         *
         * @param hash - hash-object that used as seed of ordering shuffle
         * @return shuffled cluster order
         */
        virtual nonstd::optional<ClusterOrdering> getOrdering(
            const YacHash &hash) = 0;

        virtual ~YacPeerOrderer() = default;
      };

    }  // namespace yac
  }    // namespace consensus
}  // namespace iroha

#endif  // IROHA_YAC_PEER_ORDERER_HPP

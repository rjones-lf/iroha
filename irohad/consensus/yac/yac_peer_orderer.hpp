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

#include <boost/optional.hpp>

namespace iroha {
  namespace consensus {
    namespace yac {

      class ClusterOrdering;
      class YacHash;

      /**
       * Interface responsible for creating order for yac consensus
       */
      class YacPeerOrderer {
       public:
        /**
         * Provide initial order for voting, useful when consensus initialized,
         * bot not voted before.
         * @return ordering, like in ledger
         */
        virtual boost::optional<ClusterOrdering> getInitialOrdering() = 0;

        /**
         * Provide order of peers based on hash and initial order of peers
         * @param hash - hash-object that used as seed of ordering shuffle
         * @return shuffled cluster order
         */
        virtual boost::optional<ClusterOrdering> getOrdering(
            const YacHash &hash) = 0;

        virtual ~YacPeerOrderer() = default;
      };

    }  // namespace yac
  }    // namespace consensus
}  // namespace iroha

#endif  // IROHA_YAC_PEER_ORDERER_HPP

/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_YAC_PEER_ORDERER_HPP
#define IROHA_YAC_PEER_ORDERER_HPP

#include <boost/optional.hpp>

#include "consensus/yac/cluster_order.hpp"

namespace iroha {
  namespace consensus {
    namespace yac {

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

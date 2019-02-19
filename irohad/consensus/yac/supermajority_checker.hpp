/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_CONSENSUS_SUPERMAJORITY_CHECKER_HPP
#define IROHA_CONSENSUS_SUPERMAJORITY_CHECKER_HPP

#include <memory>
#include <vector>

#include "consensus/yac/yac_types.hpp"
#include "interfaces/common_objects/range_types.hpp"
#include "interfaces/common_objects/types.hpp"

namespace shared_model {
  namespace interface {
    class Peer;
  }
}  // namespace shared_model

namespace iroha {

  namespace consensus {
    namespace yac {

      /**
       * Interface is responsible for checking if supermajority is achieved
       */
      class SupermajorityChecker {
       public:
        virtual ~SupermajorityChecker() = default;

        /**
         * Check if supermajority is achieved
         * @param signatures set of signatures to check
         * @param peers set of peers with signatures
         * @return true on supermajority is achieved or false otherwise
         */
        virtual bool hasSupermajority(
            const shared_model::interface::types::SignatureRangeType
                &signatures,
            const std::vector<std::shared_ptr<shared_model::interface::Peer>>
                &peers) const = 0;

        /**
         * Check if supermajority is possible
         * @param current actual number of signatures
         * @param all number of peers
         * @return true if supermajority is possible or false otherwise
         */
        virtual bool checkSize(PeersNumberType current,
                               PeersNumberType all) const = 0;

        /**
         * Checks if signatures is a subset of signatures of peers
         * @param signatures to check
         * @param peers with signatures
         * @return true if is subset or false otherwise
         */
        virtual bool peersSubset(
            const shared_model::interface::types::SignatureRangeType
                &signatures,
            const std::vector<std::shared_ptr<shared_model::interface::Peer>>
                &peers) const = 0;

        /**
         * Check if there is available reject proof.
         * Reject proof is proof that in current round
         * no one hash doesn't achieve supermajority.
         * @param frequent - number of times, that appears most frequent element
         * @param voted - all number of voted peers
         * @param all - number of peers in round
         * @return true, if reject
         */
        virtual bool hasReject(PeersNumberType frequent,
                               PeersNumberType voted,
                               PeersNumberType all) const = 0;
      };

    }  // namespace yac
  }    // namespace consensus
}  // namespace iroha

#endif  // IROHA_CONSENSUS_SUPERMAJORITY_CHECKER_HPP

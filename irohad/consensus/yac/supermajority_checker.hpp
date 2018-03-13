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

#ifndef IROHA_CONSENSUS_SUPERMAJORITY_CHECKER_HPP
#define IROHA_CONSENSUS_SUPERMAJORITY_CHECKER_HPP

#include <vector>
#include "interfaces/common_objects/signable_hash.hpp"

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
            const shared_model::interface::SignatureSetType &signatures,
            const std::vector<std::shared_ptr<shared_model::interface::Peer>>
                &peers) const = 0;

        /**
         * Check if supermajority is possible
         * @param current actual number of signatures
         * @param all number of peers
         * @return true if supermajority is possible or false otherwise
         */
        virtual bool checkSize(uint64_t current, uint64_t all) const = 0;

        /**
         * Checks if signatures is a subset of signatures of peers
         * @param signatures to check
         * @param peers with signatures
         * @return true if is subset or false otherwise
         */
        virtual bool peersSubset(
            const shared_model::interface::SignatureSetType &signatures,
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
        virtual bool hasReject(uint64_t frequent,
                               uint64_t voted,
                               uint64_t all) const = 0;
      };

    }  // namespace yac
  }    // namespace consensus
}  // namespace iroha

#endif  // IROHA_CONSENSUS_SUPERMAJORITY_CHECKER_HPP

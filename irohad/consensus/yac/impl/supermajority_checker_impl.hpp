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

#ifndef IROHA_SUPERMAJORITY_CHECKER_IMPL_HPP
#define IROHA_SUPERMAJORITY_CHECKER_IMPL_HPP

#include "consensus/yac/supermajority_checker.hpp"

namespace iroha {
  namespace consensus {
    namespace yac {
      class SupermajorityCheckerImpl : public SupermajorityChecker {
       public:
        virtual ~SupermajorityCheckerImpl() = default;

        /**
         * Checks if size of sets allows to have supermajority,
         * then checks subset of signatures
         * @param signatures set of votes
         * @param peers set of all peers
         * @return true on supermajority is achieved or false otherwise
         */
        virtual bool hasSupermajority(
            const shared_model::interface::SignatureSetType &signatures,
            const std::vector<std::shared_ptr<shared_model::interface::Peer>> &peers)
            const override;

        virtual bool checkSize(uint64_t current, uint64_t all) const override;

        virtual bool peersSubset(
            const shared_model::interface::SignatureSetType &signatures,
            const std::vector<std::shared_ptr<shared_model::interface::Peer>>
                &peers) const override;

        virtual bool hasReject(uint64_t frequent,
                               uint64_t voted,
                               uint64_t all) const override;
      };
    }  // namespace yac
  }    // namespace consensus
}  // namespace iroha

#endif  // IROHA_SUPERMAJORITY_CHECKER_IMPL_HPP

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

#ifndef IROHA_YAC_GATE_HPP
#define IROHA_YAC_GATE_HPP

#include "consensus/yac/cluster_order.hpp"
#include "consensus/yac/messages.hpp"
#include "consensus/yac/yac_hash_provider.hpp"
#include "network/consensus_gate.hpp"

namespace iroha {
  namespace consensus {
    namespace yac {
      class YacGate : public network::ConsensusGate {};

      /**
       * Provide gate for YAC
       * //TODO: I don't really like this class name or file name. The key point here is not about creating hashes
       * // but rather about voting as a part of the consensus process.
       */
      class HashGate {
       public:
        /**
         * Proposal new hash in network
         * @param hash - hash for voting
         */
        virtual void vote(YacHash hash, ClusterOrdering order) = 0;

        /**
         * Observable with committed hashes in network
         * @return observable for subscription
         */
        virtual rxcpp::observable<CommitMessage> on_commit() = 0;

        virtual ~HashGate() = default;
      };
    }  // namespace yac
  }    // namespace consensus
}  // namespace iroha
#endif  // IROHA_YAC_GATE_HPP

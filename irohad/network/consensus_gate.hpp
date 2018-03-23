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

#ifndef IROHA_CONSENSUS_GATE_HPP
#define IROHA_CONSENSUS_GATE_HPP

#include <model/block.hpp>
#include <rxcpp/rx.hpp>

namespace shared_model {
  namespace interface {
    class Block;
  }
}  // namespace shared_model

namespace iroha {
  namespace network {

    /**
     * Public api of consensus module
     */
    class ConsensusGate {
     public:
      /**
       * Providing data for consensus for voting
       */
      virtual void vote(const shared_model::interface::Block &) = 0;

      /**
       * Emit committed blocks
       * Note: committed block may be not satisfy for top block in ledger
       * because synchronization reasons
       */
      virtual rxcpp::observable<std::shared_ptr<shared_model::interface::Block>>
      on_commit() = 0;

      virtual ~ConsensusGate() = default;
    };
  }  // namespace network
}  // namespace iroha
#endif  // IROHA_CONSENSUS_GATE_HPP

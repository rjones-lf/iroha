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

#ifndef IROHA_YAC_GATE_IMPL_HPP
#define IROHA_YAC_GATE_IMPL_HPP

#include <memory>
#include <rxcpp/rx-observable.hpp>
#include "consensus/yac/yac_gate.hpp"
#include "consensus/yac/yac_hash_provider.hpp"
#include "logger/logger.hpp"
#include "model/block.hpp"

namespace iroha {

  namespace simulator {
    class BlockCreator;
  }

  namespace network {
    class BlockLoader;
  }

  namespace consensus {
    namespace yac {

      struct CommitMessage;
      class YacPeerOrderer;

      class YacGateImpl : public YacGate {
       public:
        YacGateImpl(std::shared_ptr<HashGate> hash_gate,
                    std::shared_ptr<YacPeerOrderer> orderer,
                    std::shared_ptr<YacHashProvider> hash_provider,
                    std::shared_ptr<simulator::BlockCreator> block_creator,
                    std::shared_ptr<network::BlockLoader> block_loader,
                    uint64_t delay);
        void vote(const shared_model::interface::Block &) override;
        rxcpp::observable<std::shared_ptr<shared_model::interface::Block>> on_commit()
            override;

       private:
        /**
         * Update current block with signatures from commit message
         * @param commit - commit message to get signatures from
         */
        void copySignatures(const CommitMessage &commit);

        std::shared_ptr<HashGate> hash_gate_;
        std::shared_ptr<YacPeerOrderer> orderer_;
        std::shared_ptr<YacHashProvider> hash_provider_;
        std::shared_ptr<simulator::BlockCreator> block_creator_;
        std::shared_ptr<network::BlockLoader> block_loader_;

        const uint64_t delay_;

        logger::Logger log_;

        std::pair<YacHash, std::shared_ptr<shared_model::interface::Block>>
            current_block_;
      };

    }  // namespace yac
  }    // namespace consensus
}  // namespace iroha

#endif  // IROHA_YAC_GATE_IMPL_HPP

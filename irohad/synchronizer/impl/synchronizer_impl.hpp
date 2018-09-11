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

#ifndef IROHA_SYNCHRONIZER_IMPL_HPP
#define IROHA_SYNCHRONIZER_IMPL_HPP

#include "ametsuchi/mutable_factory.hpp"
#include "logger/logger.hpp"
#include "network/block_loader.hpp"
#include "network/consensus_gate.hpp"
#include "synchronizer/synchronizer.hpp"
#include "validation/chain_validator.hpp"

namespace iroha {
  namespace synchronizer {
    class SynchronizerImpl : public Synchronizer {
     public:
      SynchronizerImpl(
          std::shared_ptr<network::ConsensusGate> consensus_gate,
          std::shared_ptr<validation::ChainValidator> validator,
          std::shared_ptr<ametsuchi::MutableFactory> mutableFactory,
          std::shared_ptr<network::BlockLoader> blockLoader);

      ~SynchronizerImpl() override;

      void process_commit(std::shared_ptr<shared_model::interface::Block>
                              commit_message) override;

      rxcpp::observable<SynchronizationEvent> on_commit_chain() override;

     private:
      std::shared_ptr<validation::ChainValidator> validator_;
      std::shared_ptr<ametsuchi::MutableFactory> mutable_factory_;
      std::shared_ptr<network::BlockLoader> block_loader_;

      // internal
      rxcpp::subjects::subject<SynchronizationEvent> notifier_;
      rxcpp::composite_subscription subscription_;

      logger::Logger log_;

      /**
       * Creates a temporary storage out of the provided factory
       * @return pointer to created storage
       */
      std::unique_ptr<ametsuchi::MutableStorage> createTemporaryStorage() const;

      /**
       * Process block, which can be applied to current storage directly:
       *   - apply block and commit result to Ametsuchi
       *   - notify the subscriber about commit
       * @param commit_message to be applied
       */
      void processApplicableBlock(
          std::shared_ptr<shared_model::interface::Block> commit_message) const;

      /**
       * Download part of chain, which is missed on this peer, from another; try
       * until success
       * @param commit_message - top of chain to be downloaded
       * @return observable with missed part of the chain
       */
      rxcpp::observable<std::shared_ptr<shared_model::interface::Block>>
      downloadMissingChain(
          std::shared_ptr<shared_model::interface::Block> commit_message) const;
    };
  }  // namespace synchronizer
}  // namespace iroha

#endif  // IROHA_SYNCHRONIZER_IMPL_HPP

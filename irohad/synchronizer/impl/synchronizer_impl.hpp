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
#include "network/block_loader.hpp"
#include "network/consensus_gate.hpp"
#include "synchronizer/synchronizer.hpp"
#include "validation/chain_validator.hpp"

#include "logger/logger.hpp"

namespace iroha {
  namespace synchronizer {
    class SynchronizerImpl : public Synchronizer {
     public:
      SynchronizerImpl(
          std::shared_ptr<network::ConsensusGate> consensus_gate,
          std::shared_ptr<validation::ChainValidator> validator,
          std::shared_ptr<ametsuchi::MutableFactory> mutableFactory,
          std::shared_ptr<network::BlockLoader> blockLoader);

      void process_commit(const iroha::model::Block &commit_message) override;

      rxcpp::observable<Commit> on_commit_chain() override;

     private:
      std::shared_ptr<validation::ChainValidator> validator_;
      std::shared_ptr<ametsuchi::MutableFactory> mutableFactory_;
      std::shared_ptr<network::BlockLoader> blockLoader_;

      // internal
      rxcpp::subjects::subject<Commit> notifier_;

      logger::Logger log_;
    };
  }
}

#endif  // IROHA_SYNCHRONIZER_IMPL_HPP

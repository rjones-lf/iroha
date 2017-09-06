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

#include <utility>

#include "synchronizer/impl/synchronizer_impl.hpp"

namespace iroha {
  namespace synchronizer {

    SynchronizerImpl::SynchronizerImpl(
        std::shared_ptr<network::ConsensusGate> consensus_gate,
        std::shared_ptr<validation::ChainValidator> validator,
        std::shared_ptr<ametsuchi::MutableFactory> mutableFactory,
        std::shared_ptr<network::BlockLoader> blockLoader)
        : validator_(std::move(validator)),
          mutableFactory_(std::move(mutableFactory)),
          blockLoader_(std::move(blockLoader)) {
      log_ = logger::log("synchronizer");
      consensus_gate->on_commit().subscribe([this](auto block) {
        this->process_commit(block);
      });
    }

    void SynchronizerImpl::process_commit(iroha::model::Block commit_message) {
      log_->info("processing commit");
      auto storage = mutableFactory_->createMutableStorage();
      if (not storage) {
        log_->error("Cannot create mutable storage");
        return;
      }
      if (validator_->validateBlock(commit_message, *storage)) {
        // Block can be applied to current storage
        // Commit to main Ametsuchi
        mutableFactory_->commit(std::move(storage));

        auto single_commit = rxcpp::observable<>::just(commit_message);

        notifier_.get_subscriber().on_next(single_commit);
      } else {
        // Block can't be applied to current storage
        // Download all missing blocks
        for (auto signature : commit_message.sigs) {
          storage = mutableFactory_->createMutableStorage();
          if (not storage) {
            log_->error("cannot create storage");
            return;
          }
          auto chain = blockLoader_->retrieveBlocks(signature.pubkey);
          if (validator_->validateChain(chain, *storage)) {
            // Peer send valid chain
            mutableFactory_->commit(std::move(storage));
            notifier_.get_subscriber().on_next(chain);
            // You are synchronized
            return;
          }
        }
      }
    }

    rxcpp::observable<Commit> SynchronizerImpl::on_commit_chain() {
      return notifier_.get_observable();
    }
  }
}

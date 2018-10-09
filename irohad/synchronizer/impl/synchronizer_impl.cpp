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

#include "synchronizer/impl/synchronizer_impl.hpp"

#include <utility>

#include "ametsuchi/mutable_storage.hpp"
#include "interfaces/iroha_internal/block.hpp"

namespace iroha {
  namespace synchronizer {

    SynchronizerImpl::SynchronizerImpl(
        std::shared_ptr<network::ConsensusGate> consensus_gate,
        std::shared_ptr<validation::ChainValidator> validator,
        std::shared_ptr<ametsuchi::MutableFactory> mutableFactory,
        std::shared_ptr<network::BlockLoader> blockLoader)
        : validator_(std::move(validator)),
          mutable_factory_(std::move(mutableFactory)),
          block_loader_(std::move(blockLoader)),
          log_(logger::log("synchronizer")) {
      consensus_gate->on_commit().subscribe(
          subscription_,
          [&](std::shared_ptr<shared_model::interface::Block> block) {
            this->process_commit(block);
          });
    }

    void SynchronizerImpl::process_commit(
        std::shared_ptr<shared_model::interface::Block> commit_message) {
      log_->info("processing commit");

      auto mutable_storage_var = mutable_factory_->createMutableStorage();
      if (auto e =
              boost::get<expected::Error<std::string>>(&mutable_storage_var)) {
        log_->error("could not create mutable storage: {}", e->error);
        return;
      }
      auto storage = std::move(
          boost::get<
              expected::Value<std::unique_ptr<ametsuchi::MutableStorage>>>(
              &mutable_storage_var)
              ->value);

      SynchronizationEvent result;

      if (validator_->validateBlock(commit_message, *storage)) {
        storage->apply(*commit_message);
        mutable_factory_->commit(std::move(storage));

        result = {rxcpp::observable<>::just(commit_message),
                  SynchronizationOutcomeType::kCommit};
      } else {
        auto hash = commit_message->hash();

        // while blocks are not loaded and not committed
        while (storage) {
          for (const auto &peer_signature : commit_message->signatures()) {
            auto network_chain = block_loader_->retrieveBlocks(
                shared_model::crypto::PublicKey(peer_signature.publicKey()));

            std::vector<std::shared_ptr<shared_model::interface::Block>> blocks;
            network_chain.as_blocking().subscribe(
                [&blocks](auto block) { blocks.push_back(block); });
            if (blocks.empty()) {
              log_->info("Downloaded an empty chain");
              continue;
            }

            auto chain = rxcpp::observable<>::iterate(
                blocks, rxcpp::identity_immediate());

            if (blocks.back()->hash() == hash
                and validator_->validateChain(chain, *storage)) {
              // apply downloaded chain
              for (const auto &block : blocks) {
                // we don't need to check correctness of downloaded blocks, as
                // it was done earlier on another peer
                storage->apply(*block);
              }
              mutable_factory_->commit(std::move(storage));

              result = {chain, SynchronizationOutcomeType::kCommit};
            }
          }
        }
      }

      notifier_.get_subscriber().on_next(result);
    }

    rxcpp::observable<SynchronizationEvent>
    SynchronizerImpl::on_commit_chain() {
      return notifier_.get_observable();
    }

    SynchronizerImpl::~SynchronizerImpl() {
      subscription_.unsubscribe();
    }

  }  // namespace synchronizer
}  // namespace iroha

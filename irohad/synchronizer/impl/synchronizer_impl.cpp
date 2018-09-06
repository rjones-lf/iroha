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
#include "interfaces/iroha_internal/block_variant.hpp"

namespace {
  /**
   * Lambda always returning true specially for applying blocks to storage
   */
  auto trueStorageApplyPredicate = [](const auto &, auto &, const auto &) {
    return true;
  };
}  // namespace

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
          [&](const shared_model::interface::BlockVariant &block_variant) {
            this->process_commit(block_variant);
          });
    }

    void SynchronizerImpl::process_commit(
        const shared_model::interface::BlockVariant &committed_block_variant) {
      log_->info("processing commit");
      auto storage = mutable_factory_->createMutableStorage().match(
          [](expected::Value<std::unique_ptr<ametsuchi::MutableStorage>>
                 &created_storage) { return std::move(created_storage.value); },
          [this](expected::Error<std::string> &error) {
            log_->error("could not create mutable storage: {}", error.error);
            return std::unique_ptr<ametsuchi::MutableStorage>{};
          });
      if (not storage) {
        return;
      }

      SynchronizationEvent result;

      if (validator_->validateBlock(committed_block_variant, *storage)) {
        result = iroha::visit_in_place(
            committed_block_variant,
            [&](std::shared_ptr<shared_model::interface::Block> block_ptr)
                -> SynchronizationEvent {
              storage->apply(*block_ptr, trueStorageApplyPredicate);
              mutable_factory_->commit(std::move(storage));

              return {rxcpp::observable<>::just(block_ptr),
                      SynchronizationOutcomeType::kCommit};
            },
            [&](std::shared_ptr<shared_model::interface::EmptyBlock>
                    empty_block_ptr) -> SynchronizationEvent {
              storage.reset();

              return {rxcpp::observable<>::empty<
                          std::shared_ptr<shared_model::interface::Block>>(),
                      SynchronizationOutcomeType::kCommitEmpty};
            });
      } else {
        // if committed block is not empty, it will be on top of downloaded
        // chain; otherwise, it'll contain hash of top of that chain
        auto hash = iroha::visit_in_place(
            committed_block_variant,
            [](std::shared_ptr<shared_model::interface::Block> block) {
              return block->hash();
            },
            [](std::shared_ptr<shared_model::interface::EmptyBlock> block) {
              return block->prevHash();
            });

        while (storage) {
          for (const auto &peer_signature :
               committed_block_variant.signatures()) {
            auto network_chain = block_loader_->retrieveBlocks(
                shared_model::crypto::PublicKey(peer_signature.publicKey()));

            std::vector<std::shared_ptr<shared_model::interface::Block>> blocks;
            network_chain.as_blocking().subscribe(
                [&blocks](auto block) { blocks.push_back(block); });

            auto chain = rxcpp::observable<>::iterate(
                blocks, rxcpp::identity_immediate());

            if (blocks.back()->hash() == hash
                and validator_->validateChain(chain, *storage)) {
              // apply downloaded chain
              for (const auto &block : blocks) {
                // we don't need to check correctness of downloaded blocks, as
                // it was done earlier on another peer
                storage->apply(*block, trueStorageApplyPredicate);
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

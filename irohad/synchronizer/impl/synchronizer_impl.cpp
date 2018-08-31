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

    SynchronizerImpl::~SynchronizerImpl() {
      subscription_.unsubscribe();
    }

    namespace {
      /**
       * Lambda always returning true specially for applying blocks to storage
       */
      auto trueStorageApplyPredicate = [](const auto &, auto &, const auto &) {
        return true;
      };
    }  // namespace

    std::unique_ptr<ametsuchi::MutableStorage>
    SynchronizerImpl::createTemporaryStorage() const {
      return mutable_factory_->createMutableStorage().match(
          [](expected::Value<std::unique_ptr<ametsuchi::MutableStorage>>
                 &created_storage) { return std::move(created_storage.value); },
          [this](expected::Error<std::string> &error) {
            log_->error("could not create mutable storage: {}", error.error);
            return std::unique_ptr<ametsuchi::MutableStorage>{};
          });
    }

    void SynchronizerImpl::processApplicableBlock(
        const shared_model::interface::BlockVariant &committed_block_variant)
        const {
      iroha::visit_in_place(
          committed_block_variant,
          [&](std::shared_ptr<shared_model::interface::Block> block_ptr) {
            auto storage = createTemporaryStorage();
            if (not storage) {
              return;
            }
            storage->apply(*block_ptr, trueStorageApplyPredicate);
            mutable_factory_->commit(std::move(storage));

            notifier_.get_subscriber().on_next(
                SynchronizationEvent{rxcpp::observable<>::just(block_ptr),
                                     SynchronizationOutcomeType::kCommit});
          },
          [this](std::shared_ptr<shared_model::interface::EmptyBlock>
                     empty_block_ptr) {
            notifier_.get_subscriber().on_next(SynchronizationEvent{
                rxcpp::observable<>::empty<
                    std::shared_ptr<shared_model::interface::Block>>(),
                SynchronizationOutcomeType::kCommitEmpty});
          });
    }

    rxcpp::observable<std::shared_ptr<shared_model::interface::Block>>
    SynchronizerImpl::downloadMissingChain(
        const shared_model::interface::BlockVariant &committed_block_variant)
        const {
      auto check_storage = createTemporaryStorage();
      while (true) {
        for (const auto &peer_signature :
             committed_block_variant.signatures()) {
          auto chain = block_loader_->retrieveBlocks(
              shared_model::crypto::PublicKey(peer_signature.publicKey()));
          // if committed block is not empty, it will be on top of downloaded
          // chain; otherwise, it'll contain hash of top of that chain
          auto chain_ends_with_right_block = iroha::visit_in_place(
              committed_block_variant,
              [last_downloaded_block = chain.as_blocking().last()](
                  std::shared_ptr<shared_model::interface::Block>
                      committed_block) {
                return last_downloaded_block->hash() == committed_block->hash();
              },
              [last_downloaded_block = chain.as_blocking().last()](
                  std::shared_ptr<shared_model::interface::EmptyBlock>
                      committed_empty_block) {
                return last_downloaded_block->hash()
                    == committed_empty_block->prevHash();
              });

          if (chain_ends_with_right_block
              and validator_->validateChain(chain, *check_storage)) {
            // peer sent valid chain
            return chain;
          }
        }
      }
    }

    void SynchronizerImpl::process_commit(
        const shared_model::interface::BlockVariant &committed_block_variant) {
      log_->info("processing commit");
      auto storage = createTemporaryStorage();
      if (not storage) {
        return;
      }

      if (validator_->validateBlock(committed_block_variant, *storage)) {
        processApplicableBlock(committed_block_variant);
      } else {
        auto missing_chain = downloadMissingChain(committed_block_variant);

        // TODO [IR-1634] 23.08.18 Akvinikym: place this call to notifier after
        // downloaded chain application
        notifier_.get_subscriber().on_next(SynchronizationEvent{
            missing_chain, SynchronizationOutcomeType::kCommit});

        // apply downloaded chain
        std::vector<std::shared_ptr<shared_model::interface::Block>> blocks;
        missing_chain.as_blocking().subscribe(
            [&blocks](auto block) { blocks.push_back(block); });
        for (const auto &block : blocks) {
          // we don't need to check correctness of downloaded blocks, as
          // it was done earlier on another peer
          storage->apply(*block, trueStorageApplyPredicate);
        }
        mutable_factory_->commit(std::move(storage));
      }
    }

    rxcpp::observable<SynchronizationEvent>
    SynchronizerImpl::on_commit_chain() {
      return notifier_.get_observable();
    }

  }  // namespace synchronizer
}  // namespace iroha

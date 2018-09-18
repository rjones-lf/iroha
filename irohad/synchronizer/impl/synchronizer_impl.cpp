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
        std::shared_ptr<shared_model::interface::Block> commit_message) const {
      auto storage = createTemporaryStorage();
      if (not storage) {
        return;
      }
      storage->apply(*commit_message, trueStorageApplyPredicate);
      mutable_factory_->commit(std::move(storage));

      notifier_.get_subscriber().on_next(
          SynchronizationEvent{rxcpp::observable<>::just(commit_message),
                               SynchronizationOutcomeType::kCommit});
    }

    rxcpp::observable<std::shared_ptr<shared_model::interface::Block>>
    SynchronizerImpl::downloadMissingChain(
        std::shared_ptr<shared_model::interface::Block> commit_message) const {
      auto check_storage = createTemporaryStorage();
      while (true) {
        for (const auto &peer_signature : commit_message->signatures()) {
          auto chain = block_loader_->retrieveBlocks(
              shared_model::crypto::PublicKey(peer_signature.publicKey()));
          // check that committed block is on the top of downloaded chain
          auto last_downloaded_block = chain.as_blocking().last();
          bool chain_ends_with_right_block =
              last_downloaded_block->hash() == commit_message->hash();

          if (chain_ends_with_right_block
              and validator_->validateChain(chain, *check_storage)) {
            // peer sent valid chain
            return chain;
          }
        }
      }
    }

    void SynchronizerImpl::process_commit(
        std::shared_ptr<shared_model::interface::Block> commit_message) {
      log_->info("processing commit");
      auto storage = createTemporaryStorage();
      if (not storage) {
        return;
      }

      if (validator_->validateBlock(commit_message, *storage)) {
        processApplicableBlock(commit_message);
      } else {
        auto missing_chain = downloadMissingChain(commit_message);

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

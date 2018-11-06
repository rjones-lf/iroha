/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
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
          [&](network::Commit commit) {
            this->process_commit(commit.block);
          });
    }

    SynchronizationEvent SynchronizerImpl::downloadMissingBlocks(
        std::shared_ptr<shared_model::interface::Block> commit_message,
        std::unique_ptr<ametsuchi::MutableStorage> storage) {
      auto hash = commit_message->hash();

      // while blocks are not loaded and not committed
      while (true) {
        // TODO andrei 17.10.18 IR-1763 Add delay strategy for loading blocks
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

          auto chain =
              rxcpp::observable<>::iterate(blocks, rxcpp::identity_immediate());

          if (blocks.back()->hash() == hash
              and validator_->validateAndApply(chain, *storage)) {
            mutable_factory_->commit(std::move(storage));

            return {chain, SynchronizationOutcomeType::kCommit};
          }
        }
      }
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
      auto storage =
          std::move(
              boost::get<
                  expected::Value<std::unique_ptr<ametsuchi::MutableStorage>>>(
                  mutable_storage_var))
              .value;

      auto commit = rxcpp::observable<>::just(commit_message);
      SynchronizationEvent result;

      if (validator_->validateAndApply(commit, *storage)) {
        mutable_factory_->commit(std::move(storage));

        result = {commit, SynchronizationOutcomeType::kCommit};
      } else {
        result = downloadMissingBlocks(std::move(commit_message),
                                       std::move(storage));
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

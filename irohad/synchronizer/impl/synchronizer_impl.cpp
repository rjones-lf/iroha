/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "synchronizer/impl/synchronizer_impl.hpp"

#include <utility>

#include "ametsuchi/block_query_factory.hpp"
#include "ametsuchi/mutable_storage.hpp"
#include "common/bind.hpp"
#include "interfaces/iroha_internal/block.hpp"

namespace iroha {
  namespace synchronizer {

    SynchronizerImpl::SynchronizerImpl(
        std::shared_ptr<network::ConsensusGate> consensus_gate,
        std::shared_ptr<validation::ChainValidator> validator,
        std::shared_ptr<ametsuchi::MutableFactory> mutable_factory,
        std::shared_ptr<ametsuchi::BlockQueryFactory> block_query_factory,
        std::shared_ptr<network::BlockLoader> block_loader)
        : validator_(std::move(validator)),
          mutable_factory_(std::move(mutable_factory)),
          block_query_factory_(std::move(block_query_factory)),
          block_loader_(std::move(block_loader)),
          log_(logger::log("synchronizer")) {
      consensus_gate->on_commit().subscribe(
          subscription_,
          [&](network::Commit commit) { this->process_commit(commit); });
    }

    SynchronizationEvent SynchronizerImpl::downloadMissingBlocks(
        std::shared_ptr<shared_model::interface::Block> commit_message,
        std::unique_ptr<ametsuchi::MutableStorage> storage,
        const shared_model::interface::types::HeightType height) {
      auto expected_height = commit_message->height();

      // while blocks are not loaded and not committed
      while (true) {
        // TODO andrei 17.10.18 IR-1763 Add delay strategy for loading blocks
        for (const auto &peer_signature : commit_message->signatures()) {
          auto network_chain = block_loader_->retrieveBlocks(
              height,
              shared_model::crypto::PublicKey(peer_signature.publicKey()));

          std::vector<std::shared_ptr<shared_model::interface::Block>> blocks;
          network_chain.as_blocking().subscribe(
              [&blocks](auto block) { blocks.push_back(block); });
          if (blocks.empty()) {
            log_->info("Downloaded an empty chain");
            continue;
          } else {
            log_->info("Successfully downloaded {} blocks", blocks.size());
          }

          auto chain =
              rxcpp::observable<>::iterate(blocks, rxcpp::identity_immediate());
          if (blocks.back()->height() >= expected_height
              and validator_->validateAndApply(chain, *storage)) {
            mutable_factory_->commit(std::move(storage));

            return {chain, SynchronizationOutcomeType::kCommit};
          }
        }
      }
    }

    void SynchronizerImpl::process_commit(network::Commit commit_message) {
      log_->info("processing commit");

      shared_model::interface::types::HeightType top_block_height{0};
      if (auto block_query = block_query_factory_->createBlockQuery()) {
        top_block_height = (*block_query)->getTopBlockHeight();
      } else {
        log_->error(
            "Unable to create block query and retrieve top block height");
        return;
      }

      const auto &block = commit_message.block;

      if (top_block_height >= block->height()) {
        log_->info(
            "Storage is already in synchronized state. Top block height is {}",
            top_block_height);
        return;
      }

      auto commit = rxcpp::observable<>::just(block);

      // if already voted for commit, try to apply prepared block
      if (commit_message.type == network::PeerVotedFor::kThisBlock) {
        bool block_applied = mutable_factory_->commitPrepared(*block);
        if (block_applied) {
          notifier_.get_subscriber().on_next(SynchronizationEvent{
              commit, SynchronizationOutcomeType::kCommit});
          return;
        }
      }

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

      SynchronizationEvent result;

      if (validator_->validateAndApply(commit, *storage)) {
        mutable_factory_->commit(std::move(storage));

        result = {commit, SynchronizationOutcomeType::kCommit};
      } else {
        result = downloadMissingBlocks(
            std::move(block), std::move(storage), top_block_height);
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

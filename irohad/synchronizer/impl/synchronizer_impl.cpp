/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "synchronizer/impl/synchronizer_impl.hpp"

#include <utility>

#include "ametsuchi/mutable_storage.hpp"
#include "common/visitor.hpp"
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
      consensus_gate->onOutcome().subscribe(
          subscription_, [this](consensus::GateObject object) {
            return this->processOutcome(object);
          });
    }

    void SynchronizerImpl::processOutcome(consensus::GateObject object) {
      log_->info("processing consensus outcome");
      visit_in_place(
          object,
          [this](const consensus::PairValid &msg) {
            this->processNext(msg.block, msg.round);
          },
          [this](const consensus::VoteOther &msg) {
            this->processDifferent(msg.block, msg.round);
          },
          [this](const consensus::ProposalReject &msg) {
            notifier_.get_subscriber().on_next(SynchronizationEvent{
                rxcpp::observable<>::empty<
                    std::shared_ptr<shared_model::interface::Block>>(),
                SynchronizationOutcomeType::kReject,
                msg.round});
          },
          [this](const consensus::BlockReject &msg) {
            notifier_.get_subscriber().on_next(SynchronizationEvent{
                rxcpp::observable<>::empty<
                    std::shared_ptr<shared_model::interface::Block>>(),
                SynchronizationOutcomeType::kReject,
                msg.round});
          },
          [this](const consensus::AgreementOnNone &msg) {
            notifier_.get_subscriber().on_next(SynchronizationEvent{
                rxcpp::observable<>::empty<
                    std::shared_ptr<shared_model::interface::Block>>(),
                SynchronizationOutcomeType::kNothing,
                msg.round});
          });
    }

    SynchronizationEvent SynchronizerImpl::downloadMissingBlocks(
        std::shared_ptr<shared_model::interface::Block> commit_message,
        const consensus::Round &round,
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

            return {chain, SynchronizationOutcomeType::kCommit, round};
          }
        }
      }
    }

    boost::optional<std::unique_ptr<ametsuchi::MutableStorage>>
    SynchronizerImpl::getStorage() {
      auto mutable_storage_var = mutable_factory_->createMutableStorage();
      if (auto e =
              boost::get<expected::Error<std::string>>(&mutable_storage_var)) {
        log_->error("could not create mutable storage: {}", e->error);
        return {};
      }
      return {std::move(
          boost::get<
              expected::Value<std::unique_ptr<ametsuchi::MutableStorage>>>(
              &mutable_storage_var)
              ->value)};
    }

    void SynchronizerImpl::processNext(
        std::shared_ptr<shared_model::interface::Block> commit_message,
        const consensus::Round &round) {
      log_->info("at handleNext");
      auto opt_storage = getStorage();
      if (opt_storage == boost::none) {
        return;
      }
      std::unique_ptr<ametsuchi::MutableStorage> storage =
          std::move(opt_storage.value());
      storage->apply(*commit_message);
      mutable_factory_->commit(std::move(storage));
      notifier_.get_subscriber().on_next(
          SynchronizationEvent{rxcpp::observable<>::just(commit_message),
                               SynchronizationOutcomeType::kCommit,
                               round});
    }

    void SynchronizerImpl::processDifferent(
        std::shared_ptr<shared_model::interface::Block> commit_message,
        const consensus::Round &round) {
      log_->info("at handleDifferent");
      auto opt_storage = getStorage();
      if (opt_storage == boost::none) {
        return;
      }
      std::unique_ptr<ametsuchi::MutableStorage> storage =
          std::move(opt_storage.value());
      SynchronizationEvent result =
          downloadMissingBlocks(commit_message, round, std::move(storage));
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

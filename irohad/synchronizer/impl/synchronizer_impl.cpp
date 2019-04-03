/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "synchronizer/impl/synchronizer_impl.hpp"

#include <utility>

#include "ametsuchi/block_query_factory.hpp"
#include "ametsuchi/mutable_storage.hpp"
#include "common/visitor.hpp"
#include "interfaces/iroha_internal/block.hpp"
#include "logger/logger.hpp"

namespace iroha {
  namespace synchronizer {

    SynchronizerImpl::SynchronizerImpl(
        std::shared_ptr<network::ConsensusGate> consensus_gate,
        std::shared_ptr<validation::ChainValidator> validator,
        std::shared_ptr<ametsuchi::MutableFactory> mutable_factory,
        std::shared_ptr<ametsuchi::BlockQueryFactory> block_query_factory,
        std::shared_ptr<network::BlockLoader> block_loader,
        logger::LoggerPtr log)
        : validator_(std::move(validator)),
          mutable_factory_(std::move(mutable_factory)),
          block_query_factory_(std::move(block_query_factory)),
          block_loader_(std::move(block_loader)),
          log_(std::move(log)) {
      consensus_gate->onOutcome().subscribe(
          subscription_, [this](consensus::GateObject object) {
            this->processOutcome(object);
          });
    }

    void SynchronizerImpl::processOutcome(consensus::GateObject object) {
      log_->info("processing consensus outcome");

      visit_in_place(
          object,
          [this](const consensus::PairValid &msg) { this->processNext(msg); },
          [this](const consensus::VoteOther &msg) {
            this->processDifferent(msg,
                                   false,
                                   msg.round.block_round,
                                   SynchronizationOutcomeType::kCommit);
          },
          [this](const consensus::ProposalReject &msg) {
            this->processDifferent(msg,
                                   true,
                                   msg.round.block_round - 1,
                                   SynchronizationOutcomeType::kReject);
          },
          [this](const consensus::BlockReject &msg) {
            this->processDifferent(msg,
                                   true,
                                   msg.round.block_round - 1,
                                   SynchronizationOutcomeType::kReject);
          },
          [this](const consensus::AgreementOnNone &msg) {
            this->processDifferent(msg,
                                   true,
                                   msg.round.block_round - 1,
                                   SynchronizationOutcomeType::kNothing);
          });
    }

    boost::optional<SynchronizationEvent>
    SynchronizerImpl::downloadMissingBlocks(
        const consensus::Synchronizable &msg,
        const shared_model::interface::types::HeightType top_block_height,
        const shared_model::interface::types::HeightType expected_height,
        const SynchronizationOutcomeType alternative_outcome) {
      // TODO mboldyrev 21.03.2019 IR-423 Allow consensus outcome update
      while (true) {
        // TODO andrei 17.10.18 IR-1763 Add delay strategy for loading blocks
        for (const auto &public_key : msg.public_keys) {
          auto storage = getStorage().value_or(nullptr);
          if (not storage) {
            return boost::none;
          }

          std::vector<std::shared_ptr<shared_model::interface::Block>> blocks;
          auto network_chain =
              block_loader_->retrieveBlocks(top_block_height, public_key)
                  .tap([&blocks](std::shared_ptr<shared_model::interface::Block>
                                     block) {
                    blocks.push_back(std::move(block));
                  });

          if (validator_->validateAndApply(network_chain, *storage)
              and not blocks.empty()
              and blocks.back()->height() >= expected_height) {
            auto chain = rxcpp::observable<>::iterate(
                blocks, rxcpp::identity_immediate());

            auto ledger_state = mutable_factory_->commit(std::move(storage));
            auto actual_height = blocks.back()->height();
            bool higher_than_expected = actual_height > expected_height;

            if (ledger_state) {
              return SynchronizationEvent{
                  chain,
                  higher_than_expected ? SynchronizationOutcomeType::kCommit
                                       : alternative_outcome,
                  higher_than_expected ? consensus::Round{actual_height, 0}
                                       : msg.round,
                  std::move(*ledger_state)};
              // TODO 07.03.19 andrei: IR-387 Remove reject round
            } else {
              return boost::none;
            }
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

    void SynchronizerImpl::processNext(const consensus::PairValid &msg) {
      log_->info("at handleNext");
      auto ledger_state = mutable_factory_->commitPrepared(msg.block);
      if (ledger_state) {
        notifier_.get_subscriber().on_next(
            SynchronizationEvent{rxcpp::observable<>::just(msg.block),
                                 SynchronizationOutcomeType::kCommit,
                                 msg.round,
                                 std::move(*ledger_state)});
      } else {
        auto opt_storage = getStorage();
        if (opt_storage == boost::none) {
          return;
        }
        std::unique_ptr<ametsuchi::MutableStorage> storage =
            std::move(opt_storage.value());
        if (storage->apply(msg.block)) {
          ledger_state = mutable_factory_->commit(std::move(storage));
          if (ledger_state) {
            notifier_.get_subscriber().on_next(
                SynchronizationEvent{rxcpp::observable<>::just(msg.block),
                                     SynchronizationOutcomeType::kCommit,
                                     msg.round,
                                     std::move(*ledger_state)});
          } else {
            log_->error("failed to commit mutable storage");
          }
        } else {
          log_->warn("Block was not committed due to fail in mutable storage");
        }
      }
    }

    boost::optional<shared_model::interface::types::HeightType>
    SynchronizerImpl::getTopBlockHeight() const {
      decltype(getTopBlockHeight()) top_block_height;
      if (auto block_query = block_query_factory_->createBlockQuery()) {
        top_block_height = (*block_query)->getTopBlockHeight();
      } else {
        log_->error(
            "Unable to create block query and retrieve top block height");
      }
      return top_block_height;
    }

    void SynchronizerImpl::processDifferent(
        const consensus::Synchronizable &msg,
        bool process_small_height_difference,
        shared_model::interface::types::HeightType expected_height,
        SynchronizationOutcomeType alternative_outcome) {
      log_->info("at handleDifferent");

      auto top_block_height = getTopBlockHeight();
      if (not top_block_height) {
        log_->error("Unable to continue without knowing top block height");
        return;
      }

      int64_t height_diff = msg.round.block_round - *top_block_height;
      if (height_diff < 0) {
        log_->info(
            "Storage is already in synchronized state. Top block height is {}",
            *top_block_height);
        return;
      }

      if (process_small_height_difference
          and (0 == height_diff or 1 == height_diff)) {
        notifier_.get_subscriber().on_next(
            // TODO: nickaleks IR-147 18.01.19 add peers
            // list from GateObject when it has one
            SynchronizationEvent{
                rxcpp::observable<>::empty<
                    std::shared_ptr<shared_model::interface::Block>>(),
                alternative_outcome,
                msg.round,
                nullptr /* this is going to be fixed in PR-2066 (IR-147) */});
        return;
      }

      auto result = downloadMissingBlocks(
          msg, *top_block_height, expected_height, alternative_outcome);
      if (result) {
        notifier_.get_subscriber().on_next(*result);
      }
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

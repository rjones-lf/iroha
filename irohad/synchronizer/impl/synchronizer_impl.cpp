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
            this->processDifferent(msg);
          },
          [this](const consensus::ProposalReject &msg) {
            // TODO: nickaleks IR-147 18.01.19 add peers
            // list from GateObject when it has one
            notifier_.get_subscriber().on_next(SynchronizationEvent{
                rxcpp::observable<>::empty<
                    std::shared_ptr<shared_model::interface::Block>>(),
                SynchronizationOutcomeType::kReject,
                msg.round});
          },
          [this](const consensus::BlockReject &msg) {
            // TODO: nickaleks IR-147 18.01.19 add peers
            // list from GateObject when it has one
            notifier_.get_subscriber().on_next(SynchronizationEvent{
                rxcpp::observable<>::empty<
                    std::shared_ptr<shared_model::interface::Block>>(),
                SynchronizationOutcomeType::kReject,
                msg.round});
          },
          [this](const consensus::AgreementOnNone &msg) {
            // TODO: nickaleks IR-147 18.01.19 add peers
            // list from GateObject when it has one
            notifier_.get_subscriber().on_next(SynchronizationEvent{
                rxcpp::observable<>::empty<
                    std::shared_ptr<shared_model::interface::Block>>(),
                SynchronizationOutcomeType::kNothing,
                msg.round});
          });
    }

    boost::optional<SynchronizationEvent>
    SynchronizerImpl::downloadMissingBlocks(
        const consensus::VoteOther &msg,
        std::unique_ptr<ametsuchi::MutableStorage> storage,
        const shared_model::interface::types::HeightType height) {
      auto expected_height = msg.round.block_round;

      // while blocks are not loaded and not committed
      while (true) {
        // TODO andrei 17.10.18 IR-1763 Add delay strategy for loading blocks
        for (const auto &public_key : msg.public_keys) {
          auto network_chain =
              block_loader_->retrieveBlocks(height, public_key);

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
            auto ledger_state = mutable_factory_->commit(std::move(storage));

            if (ledger_state) {
              return SynchronizationEvent{
                  chain,
                  SynchronizationOutcomeType::kCommit,
                  blocks.back()->height() > expected_height
                      // TODO 07.03.19 andrei: IR-387 Remove reject round
                      ? consensus::Round{blocks.back()->height(), 0}
                      : msg.round,
                  std::move(*ledger_state)};
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

    void SynchronizerImpl::processDifferent(const consensus::VoteOther &msg) {
      log_->info("at handleDifferent");

      shared_model::interface::types::HeightType top_block_height{0};
      if (auto block_query = block_query_factory_->createBlockQuery()) {
        top_block_height = (*block_query)->getTopBlockHeight();
      } else {
        log_->error(
            "Unable to create block query and retrieve top block height");
        return;
      }

      if (top_block_height >= msg.round.block_round) {
        log_->info(
            "Storage is already in synchronized state. Top block height is {}",
            top_block_height);
        return;
      }

      auto opt_storage = getStorage();
      if (opt_storage == boost::none) {
        return;
      }
      std::unique_ptr<ametsuchi::MutableStorage> storage =
          std::move(opt_storage.value());
      auto result =
          downloadMissingBlocks(msg, std::move(storage), top_block_height);
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

/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "synchronizer/impl/synchronizer_impl.hpp"

#include <utility>

#include "ametsuchi/block_query_factory.hpp"
#include "ametsuchi/mutable_storage.hpp"
#include "backend/protobuf/block.hpp"
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
        const shared_model::interface::types::HeightType start_height) {
      const auto target_height = msg.round.block_round;
      std::vector<std::shared_ptr<shared_model::interface::Block>> blocks;
      auto my_height = [&] { return start_height + blocks.size(); };
      auto expected_height = [&] { return my_height() + 1; };
      std::unique_ptr<ametsuchi::MutableStorage> storage;

      // TODO andrei 17.10.18 IR-1763 Add delay strategy for loading blocks
      for (const auto &public_key : msg.public_keys) {
        if (not storage) {
          storage = getStorage().value_or(nullptr);
          if (not storage) {
            return boost::none;
          }
        }
        const auto peer_start_height = my_height();
        auto network_chain =
            block_loader_->retrieveBlocks(expected_height(), public_key);

        auto subscription = rxcpp::composite_subscription();
        auto subscriber = rxcpp::make_subscriber<
            std::shared_ptr<shared_model::interface::Block>>(
            subscription,
            [&, this](std::shared_ptr<shared_model::interface::Block> block) {
              log_->debug("Received block with height {}", block->height());
              if (block->height() != expected_height()) {
                subscription.unsubscribe();
                log_->info("Received block with height {}, while expected {}",
                           block->height(),
                           expected_height());
              } else if (not validator_->validateAndApply(block, *storage)) {
                subscription.unsubscribe();
                log_->info(
                    "Could not apply block with height {} received from peer "
                    "{}. Will retry downloading from other peers.",
                    block->height(),
                    public_key);
                blocks.clear();
                storage.reset();
              } else {
                log_->debug("Successfully applied block with height {}",
                            block->height());
                blocks.push_back(block);
              }
            },
            [&, this]() {
              log_->info("Applied {} blocks from peer {}",
                         my_height() - peer_start_height,
                         public_key);
            });
        network_chain.as_blocking().subscribe(subscriber);

        auto chain =
            rxcpp::observable<>::iterate(blocks, rxcpp::identity_immediate());

        if (my_height() >= target_height) {
          auto ledger_state = mutable_factory_->commit(std::move(storage));

          if (ledger_state) {
            return SynchronizationEvent{
                chain,
                SynchronizationOutcomeType::kCommit,
                blocks.back()->height() > msg.round.block_round
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
      auto ledger_state = mutable_factory_->commitPrepared(*msg.block);
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
        if (storage->apply(*msg.block)) {
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

      auto result = downloadMissingBlocks(msg, top_block_height);
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

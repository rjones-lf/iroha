/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "simulator/impl/simulator.hpp"

#include <boost/range/adaptor/transformed.hpp>

#include "interfaces/iroha_internal/block.hpp"
#include "interfaces/iroha_internal/proposal.hpp"

namespace iroha {
  namespace simulator {

    Simulator::Simulator(
        std::shared_ptr<network::OrderingGate> ordering_gate,
        std::shared_ptr<validation::StatefulValidator> statefulValidator,
        std::shared_ptr<ametsuchi::TemporaryFactory> factory,
        std::shared_ptr<ametsuchi::BlockQueryFactory> block_query_factory,
        std::shared_ptr<shared_model::crypto::CryptoModelSigner<>>
            crypto_signer,
        std::unique_ptr<shared_model::interface::UnsafeBlockFactory>
            block_factory)
        : validator_(std::move(statefulValidator)),
          ametsuchi_factory_(std::move(factory)),
          block_query_factory_(block_query_factory),
          crypto_signer_(std::move(crypto_signer)),
          block_factory_(std::move(block_factory)),
          log_(logger::log("Simulator")) {
      ordering_gate->on_proposal().subscribe(
          proposal_subscription_,
          [this](std::shared_ptr<shared_model::interface::Proposal> proposal) {
            this->process_proposal(*proposal);
          });

      notifier_.get_observable().subscribe(
          verified_proposal_subscription_,
          [this](std::shared_ptr<iroha::validation::VerifiedProposalAndErrors>
                     verified_proposal_and_errors) {
            this->process_verified_proposal(
                *verified_proposal_and_errors->first);
          });
    }

    Simulator::~Simulator() {
      proposal_subscription_.unsubscribe();
      verified_proposal_subscription_.unsubscribe();
    }

    rxcpp::observable<
        std::shared_ptr<iroha::validation::VerifiedProposalAndErrors>>
    Simulator::on_verified_proposal() {
      return notifier_.get_observable();
    }

    void Simulator::process_proposal(
        const shared_model::interface::Proposal &proposal) {
      log_->info("process proposal");

      // Get last block from local ledger
      auto block_query_opt = block_query_factory_->createBlockQuery();
      if (not block_query_opt) {
        log_->error("could not create block query");
        return;
      }

      auto block_var = block_query_opt.value()->getTopBlock();
      if (auto e = boost::get<expected::Error<std::string>>(&block_var)) {
        log_->warn("Could not fetch last block: " + e->error);
        return;
      }

      last_block =
          boost::get<
              expected::Value<std::shared_ptr<shared_model::interface::Block>>>(
              &block_var)
              ->value;

      if (last_block->height() + 1 != proposal.height()) {
        log_->warn("Last block height: {}, proposal height: {}",
                   last_block->height(),
                   proposal.height());
        return;
      }

      auto temporary_wsv_var = ametsuchi_factory_->createTemporaryWsv();
      if (auto e =
              boost::get<expected::Error<std::string>>(&temporary_wsv_var)) {
        log_->error("could not create temporary storage: {}", e->error);
        return;
      }

      auto storage = std::move(
          boost::get<expected::Value<std::unique_ptr<ametsuchi::TemporaryWsv>>>(
              &temporary_wsv_var)
              ->value);
      auto validated_proposal_and_errors =
          std::make_shared<iroha::validation::VerifiedProposalAndErrors>(
              validator_->validate(proposal, *storage));
      storage.reset();

      notifier_.get_subscriber().on_next(
          std::move(validated_proposal_and_errors));
    }

    void Simulator::process_verified_proposal(
        const shared_model::interface::Proposal &proposal) {
      log_->info("process verified proposal");

      auto height = block_query_factory_->createBlockQuery() |
          [&](const auto &block_query) {
            return block_query->getTopBlockHeight() + 1;
          };
      if (not height) {
        log_->error("Unable to query top block height");
        return;
      }
      std::shared_ptr<shared_model::interface::Block> block =
          block_factory_->unsafeCreateBlock(height,
                                            last_block->hash(),
                                            proposal.createdTime(),
                                            proposal.transactions());
      crypto_signer_->sign(*block);
      block_notifier_.get_subscriber().on_next(block);
    }

    rxcpp::observable<std::shared_ptr<shared_model::interface::Block>>
    Simulator::on_block() {
      return block_notifier_.get_observable();
    }

  }  // namespace simulator
}  // namespace iroha

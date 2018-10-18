/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ordering/impl/on_demand_ordering_gate.hpp"

#include <numeric>

#include "common/visitor.hpp"
#include "interfaces/iroha_internal/transaction_batch_factory_impl.hpp"
#include "interfaces/iroha_internal/transaction_batch_parser_impl.hpp"

using namespace iroha;
using namespace iroha::ordering;

OnDemandOrderingGate::OnDemandOrderingGate(
    std::shared_ptr<OnDemandOrderingService> ordering_service,
    std::shared_ptr<transport::OdOsNotification> network_client,
    rxcpp::observable<BlockRoundEventType> events,
    std::shared_ptr<cache::OgCache> cache,
    std::unique_ptr<shared_model::interface::UnsafeProposalFactory> factory,
    transport::Round initial_round)
    : ordering_service_(std::move(ordering_service)),
      network_client_(std::move(network_client)),
      events_subscription_(events.subscribe([this](auto event) {
        // exclusive lock
        std::lock_guard<std::shared_timed_mutex> lock(mutex_);

        // TODO: remove parsers and factory and get batches directly from the
        // block
        shared_model::interface::TransactionBatchParserImpl batch_parser;
        shared_model::interface::TransactionBatchFactoryImpl batch_factory;

        visit_in_place(
            event,
            [this, &batch_parser, &batch_factory](
                const BlockEvent &block_event) {
              // block committed, increment block round
              current_round_ = {block_event->height(), 1};

              auto batch_transactions =
                  batch_parser.parseBatches(block_event->transactions());

              auto batches = std::accumulate(
                  batch_transactions.begin(),
                  batch_transactions.end(),
                  std::set<std::shared_ptr<
                      shared_model::interface::TransactionBatch>>(),
                  [&batch_factory](auto &batches,
                                   const auto &batch_transactions) {
                    std::vector<
                        std::shared_ptr<shared_model::interface::Transaction>>
                        vector_transaction;

                    std::transform(batch_transactions.begin(),
                                   batch_transactions.end(),
                                   std::back_inserter(vector_transaction),
                                   [](const auto &transaction) {
                                     return std::shared_ptr<
                                         std::decay_t<decltype(transaction)>>(
                                         clone(transaction));
                                   });

                    batches.insert(
                        boost::get<expected::Value<std::unique_ptr<
                            shared_model::interface::TransactionBatch>>>(
                            batch_factory.createTransactionBatch(
                                vector_transaction))
                            .value);
                    return batches;
                  });
              cache_->remove(batches);
            },
            [this](const EmptyEvent &empty) {
              // no blocks committed, increment reject round
              current_round_ = {current_round_.block_round,
                                current_round_.reject_round + 1};
            });
        cache_->up();

        // notify our ordering service about new round
        ordering_service_->onCollaborationOutcome(current_round_);

        // request proposal for the current round
        auto proposal = network_client_->onRequestProposal(current_round_);

        // vote for the object received from the network
        proposal_notifier_.get_subscriber().on_next(
            std::move(proposal).value_or_eval([&] {
              return proposal_factory_->unsafeCreateProposal(
                  current_round_.block_round, current_round_.reject_round, {});
            }));
      })),
      cache_(std::move(cache)),
      proposal_factory_(std::move(factory)),
      current_round_(initial_round) {}

void OnDemandOrderingGate::propagateBatch(
    std::shared_ptr<shared_model::interface::TransactionBatch> batch) {
  std::shared_lock<std::shared_timed_mutex> lock(mutex_);

  auto batches = cache_->clearFrontAndGet();
  batches.insert(batch);

  cache_->addToBack(batches);

  auto transactions = std::accumulate(
      batches.begin(),
      batches.end(),
      std::vector<std::shared_ptr<shared_model::interface::Transaction>>{},
      [](auto &transactions, auto batch) {
        transactions.insert(transactions.end(),
                            batch->transactions().begin(),
                            batch->transactions().end());
        return transactions;
      });

  network_client_->onTransactions(current_round_, transactions);
}

rxcpp::observable<std::shared_ptr<shared_model::interface::Proposal>>
OnDemandOrderingGate::on_proposal() {
  return proposal_notifier_.get_observable();
}

void OnDemandOrderingGate::setPcs(
    const iroha::network::PeerCommunicationService &pcs) {
  throw std::logic_error(
      "Method is deprecated. PCS observable should be set in ctor");
}

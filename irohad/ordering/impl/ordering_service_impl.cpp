/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ordering/impl/ordering_service_impl.hpp"

#include <algorithm>
#include <iterator>

#include <boost/range/adaptor/indirected.hpp>

#include "ametsuchi/ordering_service_persistent_state.hpp"
#include "datetime/time.hpp"
#include "network/ordering_service_transport.hpp"

namespace iroha {
  namespace ordering {
    OrderingServiceImpl::OrderingServiceImpl(
        std::shared_ptr<ametsuchi::PeerQueryFactory> peer_query_factory,
        size_t max_size,
        rxcpp::observable<TimeoutType> proposal_timeout,
        std::shared_ptr<network::OrderingServiceTransport> transport,
        std::shared_ptr<ametsuchi::OsPersistentStateFactory> persistent_state,
        std::unique_ptr<shared_model::interface::ProposalFactory> factory,
        bool is_async)
        : peer_query_factory_(peer_query_factory),
          max_size_(max_size),
          current_size_(0),
          transport_(transport),
          persistent_state_(persistent_state),
          factory_(std::move(factory)),
          proposal_height_(persistent_state_->createOsPersistentState() |
                           [](const auto &state) {
                             return state->loadProposalHeight().value();
                           }),
          log_(logger::log("OrderingServiceImpl")) {
      // restore state of ordering service from persistent storage
      rxcpp::observable<ProposalEvent> timer =
          proposal_timeout.map([](auto) { return ProposalEvent::kTimerEvent; });

      auto subscribe = [&](auto merge_strategy) {
        handle_ = merge_strategy(rxcpp::observable<>::from(
                                     timer, transactions_.get_observable()))
                      .subscribe([this](auto &&v) {
                        auto check_queue = [&] {
                          switch (v) {
                            case ProposalEvent::kTimerEvent:
                              return not queue_.empty();
                            case ProposalEvent::kBatchEvent:
                              return current_size_.load() >= max_size_;
                            default:
                              BOOST_ASSERT_MSG(false, "Unknown value");
                          }
                        };
                        if (check_queue()) {
                          this->generateProposal();
                        }
                      });
      };

      if (is_async) {
        subscribe([](auto observable) {
          return observable.merge(rxcpp::synchronize_new_thread());
        });
      } else {
        subscribe([](auto observable) { return observable.merge(); });
      }
    }

    void OrderingServiceImpl::onBatch(
        shared_model::interface::TransactionBatch &&batch) {
      std::shared_lock<std::shared_timed_mutex> batch_prop_lock(
          batch_prop_mutex_);

      current_size_.fetch_add(batch.transactions().size());
      queue_.push(std::make_unique<shared_model::interface::TransactionBatch>(
          std::move(batch)));
      log_->info("Queue size is {}", current_size_.load());

      batch_prop_lock.unlock();

      std::lock_guard<std::mutex> event_lock(event_mutex_);
      transactions_.get_subscriber().on_next(ProposalEvent::kBatchEvent);
    }

    void OrderingServiceImpl::generateProposal() {
      std::lock_guard<std::shared_timed_mutex> lock(batch_prop_mutex_);
      log_->info("Start proposal generation");
      std::vector<std::shared_ptr<shared_model::interface::Transaction>> txs;
      for (std::unique_ptr<shared_model::interface::TransactionBatch> batch;
           txs.size() < max_size_ and queue_.try_pop(batch);) {
        auto batch_size = batch->transactions().size();
        // TODO 29.08.2018 andrei IR-1667 Timestamp validation during proposal
        // generation
        txs.insert(std::end(txs),
                   std::make_move_iterator(std::begin(batch->transactions())),
                   std::make_move_iterator(std::end(batch->transactions())));
        current_size_ -= batch_size;
      }

      auto tx_range = txs | boost::adaptors::indirected;
      auto proposal = factory_->createProposal(
          proposal_height_++, iroha::time::now(), tx_range);

      proposal.match(
          [this](expected::Value<
                 std::unique_ptr<shared_model::interface::Proposal>> &v) {
            // Save proposal height to the persistent storage.
            // In case of restart it reloads state.
            if (persistent_state_->createOsPersistentState() |
                [this](const auto &state) {
                  return state->saveProposalHeight(proposal_height_);
                }) {
              publishProposal(std::move(v.value));
            } else {
              // TODO(@l4l) 23/03/18: publish proposal independent of psql
              // status IR-1162
              log_->warn(
                  "Proposal height cannot be saved. Skipping proposal publish");
            }
          },
          [this](expected::Error<std::string> &e) {
            log_->warn("Failed to initialize proposal: {}", e.error);
          });
    }

    void OrderingServiceImpl::publishProposal(
        std::unique_ptr<shared_model::interface::Proposal> proposal) {
      auto peers = peer_query_factory_->createPeerQuery() |
          [](const auto &query) { return query->getLedgerPeers(); };
      if (peers) {
        std::vector<std::string> addresses;
        std::transform(peers->begin(),
                       peers->end(),
                       std::back_inserter(addresses),
                       [](auto &p) { return p->address(); });
        transport_->publishProposal(std::move(proposal), addresses);
      } else {
        log_->error("Cannot get the peer list");
      }
    }

    OrderingServiceImpl::~OrderingServiceImpl() {
      handle_.unsubscribe();
    }
  }  // namespace ordering
}  // namespace iroha

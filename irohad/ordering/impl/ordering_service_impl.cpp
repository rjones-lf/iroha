/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ordering/impl/ordering_service_impl.hpp"

#include <algorithm>
#include <iterator>

#include "ametsuchi/ordering_service_persistent_state.hpp"
#include "ametsuchi/peer_query.hpp"
#include "backend/protobuf/proposal.hpp"
#include "backend/protobuf/transaction.hpp"
#include "datetime/time.hpp"
#include "network/ordering_service_transport.hpp"

namespace iroha {
  namespace ordering {
    OrderingServiceImpl::OrderingServiceImpl(
        std::shared_ptr<ametsuchi::PeerQuery> wsv,
        size_t max_size,
        rxcpp::observable<TimeoutType> proposal_timeout,
        std::shared_ptr<network::OrderingServiceTransport> transport,
        std::shared_ptr<ametsuchi::OrderingServicePersistentState>
            persistent_state,
        bool is_async)
        : wsv_(wsv),
          max_size_(max_size),
          current_size_(0),
          transport_(transport),
          persistent_state_(persistent_state){
      log_ = logger::log("OrderingServiceImpl");

      // restore state of ordering service from persistent storage
      proposal_height_ = persistent_state_->loadProposalHeight().value();

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

      // TODO 05/03/2018 andrei IR-1046 Server-side shared model object
      // factories with move semantics
      iroha::protocol::Proposal proto_proposal;
      proto_proposal.set_height(proposal_height_++);
      proto_proposal.set_created_time(iroha::time::now());
      log_->info("Start proposal generation");
      for (std::unique_ptr<shared_model::interface::TransactionBatch> batch;
           static_cast<size_t>(proto_proposal.transactions_size()) < max_size_
           and queue_.try_pop(batch);) {
        std::for_each(
            batch->transactions().begin(),
            batch->transactions().end(),
            [this, &proto_proposal](auto &tx) {
              *proto_proposal.add_transactions() =
                  std::static_pointer_cast<shared_model::proto::Transaction>(tx)
                      ->getTransport();
              current_size_--;
            });
      }

      auto proposal = std::make_unique<shared_model::proto::Proposal>(
          std::move(proto_proposal));

      // Save proposal height to the persistent storage.
      // In case of restart it reloads state.
      if (persistent_state_->saveProposalHeight(proposal_height_)) {
        publishProposal(std::move(proposal));
      } else {
        // TODO(@l4l) 23/03/18: publish proposal independent of psql status
        // IR-1162
        log_->warn(
            "Proposal height cannot be saved. Skipping proposal publish");
      }
    }

    void OrderingServiceImpl::publishProposal(
        std::unique_ptr<shared_model::interface::Proposal> proposal) {
      auto peers = wsv_->getLedgerPeers();
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

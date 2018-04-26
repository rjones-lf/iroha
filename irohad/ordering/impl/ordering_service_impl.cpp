/**
 * Copyright Soramitsu Co., Ltd. 2018 All Rights Reserved.
 * http://soramitsu.co.jp
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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
        size_t delay_milliseconds,
        std::shared_ptr<network::OrderingServiceTransport> transport,
        std::shared_ptr<ametsuchi::OrderingServicePersistentState>
            persistent_state,
        bool is_async)
        : wsv_(wsv),
          max_size_(max_size),
          delay_milliseconds_(delay_milliseconds),
          transport_(transport),
          persistent_state_(persistent_state) {
      log_ = logger::log("OrderingServiceImpl");

      // restore state of ordering service from persistent storage
      proposal_height_ = persistent_state_->loadProposalHeight().value();

      auto subscribe = [&](auto merge_strategy) {
        auto interval = std::chrono::milliseconds(delay_milliseconds_);

        rxcpp::observable<ProposalEvent> timer =
            rxcpp::observable<>::interval(
                std::chrono::steady_clock::now() + interval,
                interval,
                rxcpp::observe_on_new_thread())
                .map([](auto) { return ProposalEvent::kTimerEvent; });

        handle_ = merge_strategy(rxcpp::observable<>::from(
                                     timer, transactions_.get_observable()))
                      .subscribe([this](auto &&v) {
                        auto check_queue = [&] {
                          switch (v) {
                            case ProposalEvent::kTimerEvent:
                              return not queue_.empty();
                            case ProposalEvent::kTransactionEvent:
                              return queue_.unsafe_size() >= max_size_;
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

    void OrderingServiceImpl::onTransaction(
        std::shared_ptr<shared_model::interface::Transaction> transaction) {
      queue_.push(transaction);
      log_->info("Queue size is {}", queue_.unsafe_size());

      // on_next calls should not be concurrent
      std::lock_guard<std::mutex> lk(mutex_);
      transactions_.get_subscriber().on_next(ProposalEvent::kTransactionEvent);
    }

    void OrderingServiceImpl::generateProposal() {
      // TODO 05/03/2018 andrei IR-1046 Server-side shared model object
      // factories with move semantics
      iroha::protocol::Proposal proto_proposal;
      proto_proposal.set_height(proposal_height_++);
      proto_proposal.set_created_time(iroha::time::now());
      log_->info("Start proposal generation");
      for (std::shared_ptr<shared_model::interface::Transaction> tx;
           static_cast<size_t>(proto_proposal.transactions_size()) < max_size_
           and queue_.try_pop(tx);) {
        *proto_proposal.add_transactions() =
            std::move(static_cast<shared_model::proto::Transaction *>(tx.get())
                          ->getTransport());
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

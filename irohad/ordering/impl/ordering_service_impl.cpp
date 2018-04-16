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
        rxcpp::observable<Rep> proposal_timeout,
        std::shared_ptr<network::OrderingServiceTransport> transport,
        std::shared_ptr<ametsuchi::OrderingServicePersistentState>
            persistent_state)
        : wsv_(wsv),
          max_size_(max_size),
          transport_(transport),
          persistent_state_(persistent_state),
          is_finished(false) {
      log_ = logger::log("OrderingServiceImpl");
      // restore state of ordering service from persistent storage
      proposal_height = persistent_state_->loadProposalHeight().value();

      handle =
          proposal_timeout.take_while([this](auto) { return !is_finished; })
              .subscribe_on(rxcpp::observe_on_new_thread())
              .subscribe([this](auto) { this->updateTimer(); });
    }

    void OrderingServiceImpl::onTransaction(
        std::shared_ptr<shared_model::interface::Transaction> transaction) {
      queue_.push(transaction);
      log_->info("Queue size is {}", queue_.unsafe_size());

      if (queue_.unsafe_size() >= max_size_) {
        updateTimer();
      }
    }

    void OrderingServiceImpl::generateProposal() {
      // TODO 05/03/2018 andrei IR-1046 Server-side shared model object
      // factories with move semantics
      iroha::protocol::Proposal proto_proposal;
      proto_proposal.set_height(proposal_height++);
      proto_proposal.set_created_time(iroha::time::now());
      log_->info("Start proposal generation");
      for (std::shared_ptr<shared_model::interface::Transaction> tx;
           static_cast<size_t>(proto_proposal.transactions_size()) < max_size_
           and queue_.try_pop(tx);) {
        *proto_proposal.add_transactions() = std::move(
            std::static_pointer_cast<shared_model::proto::Transaction>(tx)
                ->getTransport());
      }

      auto proposal = std::make_unique<shared_model::proto::Proposal>(
          std::move(proto_proposal));

      // Save proposal height to the persistent storage.
      // In case of restart it reloads state.
      if (persistent_state_->saveProposalHeight(proposal_height)) {
        publishProposal(std::move(proposal));
      } else {
        // TODO(@l4l) 23/03/18: publish proposal independant of psql status
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

    void OrderingServiceImpl::updateTimer() {
      std::lock_guard<std::mutex> lock(m);
      if (is_finished) {
        return;
      }
      if (not queue_.empty()) {
        this->generateProposal();
      }
    }

    OrderingServiceImpl::~OrderingServiceImpl() {
      is_finished = true;
      std::lock_guard<std::mutex> lock(m);
    }
  }  // namespace ordering
}  // namespace iroha

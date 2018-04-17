/**
 * Copyright Soramitsu Co., Ltd. 2017 All Rights Reserved.
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

#include <utility>

#include "ordering/impl/ordering_gate_impl.hpp"

#include "interfaces/iroha_internal/proposal.hpp"
#include "interfaces/transaction.hpp"
#include "interfaces/iroha_internal/block.hpp"

namespace iroha {
  namespace ordering {

    bool ProposalComparator::operator()(
        const std::shared_ptr<shared_model::interface::Proposal> &lhs,
        const std::shared_ptr<shared_model::interface::Proposal> &rhs) const {
      return lhs->height() > rhs->height();
    }

    OrderingGateImpl::OrderingGateImpl(
        std::shared_ptr<iroha::network::OrderingGateTransport> transport)
        : transport_(std::move(transport)), log_(logger::log("OrderingGate")), last_block_height(0) {}

    void OrderingGateImpl::propagateTransaction(
        std::shared_ptr<const shared_model::interface::Transaction>
            transaction) {
      log_->info("propagate tx, account_id: {}",
                 " account_id: " + transaction->creatorAccountId());

      transport_->propagateTransaction(transaction);
    }

    rxcpp::observable<std::shared_ptr<shared_model::interface::Proposal>>
    OrderingGateImpl::on_proposal() {
      return proposals_.get_observable();
    }

    void OrderingGateImpl::setPcs(
        const iroha::network::PeerCommunicationService &pcs) {
      pcs_subscriber_ = pcs.on_commit().subscribe([this](const auto &block) {
        // TODO: 05/03/2018 @muratovv rework behavior of queue with respect to
        // block height IR-1042
        block.subscribe([this](const auto &b) {
          unlock_next_.store(true);
          this->last_block_height = b->height();
          this->tryNextRound();
        });
      });
    }

    void OrderingGateImpl::onProposal(
        std::shared_ptr<shared_model::interface::Proposal> proposal) {
      log_->info("Received new proposal");
      proposal_queue_.push(std::move(proposal));
      tryNextRound();
    }

    void OrderingGateImpl::tryNextRound() {
      while (not proposal_queue_.empty() and unlock_next_) {
        std::shared_ptr<shared_model::interface::Proposal> next_proposal;
        proposal_queue_.try_pop(next_proposal);
        // check for out of order proposal
        if (next_proposal->height() < last_block_height + 1) {
          log_->info("Old proposal, discarding");
          continue;
        }
        log_->info("Pass the proposal to pipeline");
        unlock_next_.store(false);
//        last_proposal = next_proposal->height();
        proposals_.get_subscriber().on_next(next_proposal);
      }
    }

    OrderingGateImpl::~OrderingGateImpl() {
      pcs_subscriber_.unsubscribe();
    }

  }  // namespace ordering
}  // namespace iroha

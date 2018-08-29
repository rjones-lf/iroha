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

#include <tuple>
#include <utility>

#include "ordering/impl/ordering_gate_impl.hpp"

#include "interfaces/iroha_internal/block.hpp"
#include "interfaces/iroha_internal/proposal.hpp"
#include "interfaces/transaction.hpp"

namespace iroha {
  namespace ordering {

    bool ProposalComparator::operator()(
        const std::shared_ptr<shared_model::interface::Proposal> &lhs,
        const std::shared_ptr<shared_model::interface::Proposal> &rhs) const {
      return lhs->height() > rhs->height();
    }

    OrderingGateImpl::OrderingGateImpl(
        std::shared_ptr<iroha::network::OrderingGateTransport> transport,
        shared_model::interface::types::HeightType initial_height,
        bool run_async)
        : transport_(std::move(transport)),
          last_block_height_(initial_height),
          log_(logger::log("OrderingGate")),
          run_async_(run_async) {}

    void OrderingGateImpl::propagateTransaction(
        std::shared_ptr<const shared_model::interface::Transaction> transaction)
        const {
      log_->info("propagate tx, account_id: {}",
                 " account_id: " + transaction->creatorAccountId());

      transport_->propagateTransaction(transaction);
    }

    void OrderingGateImpl::propagateBatch(
        const shared_model::interface::TransactionBatch &batch) const {
      if (batch.transactions().empty()) {
        log_->warn("trying to propagate empty batch");
        return;
      }
      log_->info("propagate batch, account_id: {}",
                 batch.transactions().front()->creatorAccountId());

      transport_->propagateBatch(batch);
    }

    rxcpp::observable<std::shared_ptr<shared_model::interface::Proposal>>
    OrderingGateImpl::on_proposal() {
      return proposals_.get_observable();
    }

    void OrderingGateImpl::setPcs(
        const iroha::network::PeerCommunicationService &pcs) {
      log_->info("setPcs");

      /// observable which contains heights of the top committed blocks
      auto top_block_height =
          pcs.on_commit()
              .transform(
                  [this](const synchronizer::SynchronizationEvent &sync_event) {
                    sync_event.synced_blocks.subscribe(
                        // take height of next block
                        [this](std::shared_ptr<shared_model::interface::Block>
                                   block_ptr) {
                          last_block_height_ = block_ptr->height();
                        });
                    return last_block_height_;
                  })
              .start_with(last_block_height_);

      /// merge_strategy - observable with another source of block heights
      auto subscribe = [&](auto merge_strategy) {
        pcs_subscriber_ = merge_strategy(net_proposals_.get_observable())
                              .subscribe([this](const auto &t) {
                                // t is zip of two observables, there is
                                // intentionally ignored first value (with stub
                                // values) because it is required only for
                                // synchronization
                                this->tryNextRound(std::get<1>(t));
                              });
      };

      if (run_async_) {
        subscribe([&top_block_height](auto observable) {
          return observable.combine_latest(rxcpp::synchronize_new_thread(),
                                           top_block_height);
        });
      } else {
        subscribe([&top_block_height](auto observable) {
          return observable.combine_latest(top_block_height);
        });
      }
    }

    void OrderingGateImpl::onProposal(
        std::shared_ptr<shared_model::interface::Proposal> proposal) {
      log_->info("Received new proposal, height: {}", proposal->height());
      proposal_queue_.push(std::move(proposal));
      std::lock_guard<std::mutex> lock(proposal_mutex_);
      // intentionally pass stub value
      net_proposals_.get_subscriber().on_next(0);
    }

    void OrderingGateImpl::tryNextRound(
        shared_model::interface::types::HeightType last_block_height) {
      log_->debug("TryNextRound");
      std::shared_ptr<shared_model::interface::Proposal> next_proposal;
      while (proposal_queue_.try_pop(next_proposal)) {
        // check for old proposal
        if (next_proposal->height() < last_block_height + 1) {
          log_->debug("Old proposal, discarding");
          continue;
        }
        // check for new proposal
        if (next_proposal->height() > last_block_height + 1) {
          log_->debug("Proposal newer than last block, keeping in queue");
          proposal_queue_.push(next_proposal);
          break;
        }
        log_->info("Pass the proposal to pipeline height {}",
                   next_proposal->height());
        proposals_.get_subscriber().on_next(next_proposal);
      }
    }

    OrderingGateImpl::~OrderingGateImpl() {
      pcs_subscriber_.unsubscribe();
    }

  }  // namespace ordering
}  // namespace iroha

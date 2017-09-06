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

#include "simulator/impl/simulator.hpp"

namespace iroha {
  namespace simulator {

    Simulator::Simulator(
        std::shared_ptr<network::OrderingGate> ordering_gate,
        std::shared_ptr<validation::StatefulValidator> statefulValidator,
        std::shared_ptr<ametsuchi::TemporaryFactory> factory,
        std::shared_ptr<ametsuchi::BlockQuery> blockQuery,
        std::shared_ptr<model::HashProviderImpl> hash_provider)
        : validator_(std::move(statefulValidator)),
          ametsuchi_factory_(std::move(factory)),
          block_queries_(std::move(blockQuery)),
          hash_provider_(std::move(hash_provider)) {
      log_ = logger::log("Simulator");
      ordering_gate->on_proposal().subscribe(
          [this](auto proposal) { this->process_proposal(proposal); });

      notifier_.get_observable().subscribe([this](auto verified_proposal) {
        this->process_verified_proposal(verified_proposal);
      });
    }

    rxcpp::observable<model::Proposal> Simulator::on_verified_proposal() {
      return notifier_.get_observable();
    }

    void Simulator::process_proposal(model::Proposal proposal) {
      log_->info("process proposal");
      // Get last block from local ledger
      block_queries_->getTopBlocks(1)
          .as_blocking()
          .subscribe([this](auto block) {
            last_block = block;
          });
      if (not last_block.has_value() or
          last_block.value().height + 1 != proposal.height) {
        return;
      }
      auto temporaryStorage = ametsuchi_factory_->createTemporaryWsv();
      auto verified_proposal =
          validator_->validate(proposal, *temporaryStorage);
      if (not verified_proposal.transactions.empty()) {
        notifier_.get_subscriber().on_next(verified_proposal);
      }
    }

    void Simulator::process_verified_proposal(model::Proposal proposal) {
      log_->info("process verified proposal");
      model::Block new_block;
      new_block.height = proposal.height;
      new_block.prev_hash = last_block.value().hash;
      new_block.transactions = proposal.transactions;
      new_block.txs_number = proposal.transactions.size();
      new_block.created_ts = 0; // todo set timestamp from proposal
      new_block.merkle_root.fill(0); // todo make effective impl
      new_block.hash = hash_provider_->get_hash(new_block);
      new_block.sigs.push_back({});

      block_notifier_.get_subscriber().on_next(new_block);
    }

    rxcpp::observable<model::Block> Simulator::on_block() {
      return block_notifier_.get_observable();
    }

  }  // namespace simulator
}  // namespace iroha

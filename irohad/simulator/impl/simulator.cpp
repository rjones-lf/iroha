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

    Simulator::Simulator(validation::StatefulValidator& statefulValidator,
                         ametsuchi::TemporaryFactory& factory,
                         ametsuchi::BlockQuery& blockQuery,
                         model::HashProviderImpl& hash_provider)
        : validator_(statefulValidator),
          ametsuchi_factory_(factory),
          block_queries_(blockQuery),
          hash_provider_(hash_provider) {}

    rxcpp::observable<nonstd::optional<model::Proposal>>
    Simulator::on_verified_proposal() {
      return notifier_.get_observable();
    }

    void Simulator::process_proposal(model::Proposal proposal) {
      auto temporaryStorage = ametsuchi_factory_.createTemporaryWsv();
      auto current_height = proposal.height;
      bool no_block = true;
      // Get last block from local ledger
      block_queries_.getBlocks(current_height - 1, current_height)
          .as_blocking()
          .subscribe([&no_block, this](auto block) {
            no_block = false;
            this->last_block = block;
          });
      if (no_block) {
        // You are not up-to-date
        notifier_.get_subscriber().on_next(nonstd::nullopt);
      } else {
        // You are up-to-date, start stateful valdiation
        notifier_.get_subscriber().on_next(
            validator_.validate(proposal, *temporaryStorage));
      }
      notifier_.get_observable().subscribe([this](auto verified_proposal) {
        this->process_verified_proposal(verified_proposal);
      });
    }

    void Simulator::process_verified_proposal(
        nonstd::optional<model::Proposal> proposal) {
      if (proposal) {
        model::Block new_block = model::Block();
        new_block.height = last_block.height + 1;
        new_block.prev_hash = last_block.hash;
        new_block.transactions = proposal.value().transactions;
        new_block.txs_number = proposal.value().transactions.size();
        new_block.hash = hash_provider_.get_hash(new_block);
        block_notifier_.get_subscriber().on_next(new_block);
      } else {
        // Verified proposal was not formed
        block_notifier_.get_subscriber().on_next(nonstd::nullopt);
      }
    }

    rxcpp::observable<nonstd::optional<model::Block>> Simulator::on_block() {
      return block_notifier_.get_observable();
    }

  }  // namespace simulator
}  // namespace iroha
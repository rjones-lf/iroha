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
#include "builders/protobuf/block.hpp"
#include "interfaces/iroha_internal/block.hpp"
#include "interfaces/iroha_internal/proposal.hpp"
#include "model/sha3_hash.hpp"

#include "backend/protobuf/from_old_model.hpp"

namespace iroha {
  namespace simulator {

    Simulator::Simulator(
        std::shared_ptr<network::OrderingGate> ordering_gate,
        std::shared_ptr<validation::StatefulValidator> statefulValidator,
        std::shared_ptr<ametsuchi::TemporaryFactory> factory,
        std::shared_ptr<ametsuchi::BlockQuery> blockQuery,
        std::shared_ptr<model::ModelCryptoProvider> crypto_provider)
        : validator_(std::move(statefulValidator)),
          ametsuchi_factory_(std::move(factory)),
          block_queries_(std::move(blockQuery)),
          crypto_provider_(std::move(crypto_provider)) {
      log_ = logger::log("Simulator");
      ordering_gate->on_proposal().subscribe(
          proposal_subscription_,
          [this](std::shared_ptr<shared_model::interface::Proposal> proposal) {
            this->process_proposal(*proposal);
          });

      notifier_.get_observable().subscribe(
          verified_proposal_subscription_,
          [this](const std::shared_ptr<shared_model::interface::Proposal>
                     &verified_proposal) {
            this->process_verified_proposal(*verified_proposal);
          });
    }

    Simulator::~Simulator() {
      proposal_subscription_.unsubscribe();
      verified_proposal_subscription_.unsubscribe();
    }

    rxcpp::observable<std::shared_ptr<shared_model::interface::Proposal>>
    Simulator::on_verified_proposal() {
      return notifier_.get_observable();
    }

    void Simulator::process_proposal(
        const shared_model::interface::Proposal &proposal) {
      log_->info("process proposal");
      // Get last block from local ledger
      block_queries_->getTopBlocks(1).as_blocking().subscribe(
          [this](auto block) { last_block = block; });
      if (not last_block) {
        log_->warn("Could not fetch last block");
        return;
      }
      if (last_block.value()->height() + 1 != proposal.height()) {
        log_->warn("Last block height: {}, proposal height: {}",
                   last_block.value()->height(),
                   proposal.height());
        return;
      }
      auto temporaryStorageResult = ametsuchi_factory_->createTemporaryWsv();
      temporaryStorageResult.match(
          [&](expected::Value<std::unique_ptr<ametsuchi::TemporaryWsv>>
                  &temporaryStorage) {
            auto validated_proposal =
                validator_->validate(proposal, *temporaryStorage.value);
            notifier_.get_subscriber().on_next(validated_proposal);
          },
          [&](expected::Error<std::string> &error) {
            log_->error(error.error);
            // TODO: 13/02/18 Solonets - Handle the case when TemporaryWsv was
            // failed to produced - IR-966
            throw std::runtime_error(error.error);
          });
    }

    void Simulator::process_verified_proposal(
        const shared_model::interface::Proposal &proposal) {
      log_->info("process verified proposal");

      model::Block new_block;
      new_block.height = proposal.height();
      new_block.prev_hash =
          *iroha::hexstringToArray<iroha::model::Block::HashType::size()>(
              last_block.value()->hash().hex());
      auto txs = boost::accumulate(
          proposal.transactions(),
          std::vector<iroha::model::Transaction>{},
          [](auto &&vec, const auto &tx) {
            std::unique_ptr<iroha::model::Transaction> ptr(tx->makeOldModel());
            vec.emplace_back(*ptr);
            return std::forward<decltype(vec)>(vec);
          });
      new_block.transactions = txs;
      new_block.txs_number = proposal.transactions().size();
      new_block.created_ts = proposal.createdTime();
      new_block.hash = hash(new_block);
      crypto_provider_->sign(new_block);

      auto block = std::make_shared<shared_model::proto::Block>(
          shared_model::proto::Block(shared_model::proto::from_old(new_block)));
      block_notifier_.get_subscriber().on_next(block);
    }

    rxcpp::observable<std::shared_ptr<shared_model::interface::Block>>
    Simulator::on_block() {
      return block_notifier_.get_observable();
    }

  }  // namespace simulator
}  // namespace iroha

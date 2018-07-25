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

#include <boost/range/adaptor/transformed.hpp>

#include "backend/protobuf/empty_block.hpp"
#include "builders/protobuf/block.hpp"
#include "builders/protobuf/empty_block.hpp"
#include "interfaces/iroha_internal/block.hpp"
#include "interfaces/iroha_internal/proposal.hpp"

namespace iroha {
  namespace simulator {

    Simulator::Simulator(
        std::shared_ptr<network::OrderingGate> ordering_gate,
        std::shared_ptr<validation::StatefulValidator> statefulValidator,
        std::shared_ptr<ametsuchi::TemporaryFactory> factory,
        std::shared_ptr<ametsuchi::BlockQuery> blockQuery,
        std::shared_ptr<shared_model::crypto::CryptoModelSigner<>>
            crypto_signer)
        : validator_(std::move(statefulValidator)),
          ametsuchi_factory_(std::move(factory)),
          block_queries_(std::move(blockQuery)),
          crypto_signer_(std::move(crypto_signer)) {
      log_ = logger::log("Simulator");
      ordering_gate->on_proposal().subscribe(
          proposal_subscription_,
          [this](std::shared_ptr<shared_model::interface::Proposal> proposal) {
            this->process_proposal(*proposal);
          });

      notifier_.get_observable().subscribe(
          verified_proposal_subscription_,
          [this](std::shared_ptr<iroha::validation::VerifiedProposalAndErrors>
                     verified_proposal_and_errors) {
            this->process_verified_proposal(
                *verified_proposal_and_errors->first);
          });
    }

    Simulator::~Simulator() {
      proposal_subscription_.unsubscribe();
      verified_proposal_subscription_.unsubscribe();
    }

    rxcpp::observable<
        std::shared_ptr<iroha::validation::VerifiedProposalAndErrors>>
    Simulator::on_verified_proposal() {
      return notifier_.get_observable();
    }

    void Simulator::process_proposal(
        const shared_model::interface::Proposal &proposal) {
      log_->info("process proposal");
      // Get last block from local ledger
      auto top_block_result = block_queries_->getTopBlock();
      auto block_fetched = top_block_result.match(
          [&](expected::Value<std::shared_ptr<shared_model::interface::Block>>
                  &block) {
            last_block = block.value;
            return true;
          },
          [this](expected::Error<std::string> &error) {
            log_->warn("Could not fetch last block: " + error.error);
            return false;
          });
      if (not block_fetched) {
        return;
      }

      if (last_block->height() + 1 != proposal.height()) {
        log_->warn("Last block height: {}, proposal height: {}",
                   last_block->height(),
                   proposal.height());
        return;
      }
      auto temporaryStorageResult = ametsuchi_factory_->createTemporaryWsv();
      temporaryStorageResult.match(
          [&](expected::Value<std::unique_ptr<ametsuchi::TemporaryWsv>>
                  &temporaryStorage) {
            auto validated_proposal_and_errors =
                std::make_shared<iroha::validation::VerifiedProposalAndErrors>(
                    validator_->validate(proposal, *temporaryStorage.value));
            notifier_.get_subscriber().on_next(
                std::move(validated_proposal_and_errors));
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

      auto height = block_queries_->getTopBlockHeight() + 1;
      auto block = block_factory_->unsafeCreateBlock(height,
                                                     last_block->hash(),
                                                     proposal.createdTime(),
                                                     proposal.transactions());

      crypto_signer_->sign(block);
      block_notifier_.get_subscriber().on_next(block);
    }

    rxcpp::observable<shared_model::interface::BlockVariant>
    Simulator::on_block() {
      return block_notifier_.get_observable();
    }

  }  // namespace simulator
}  // namespace iroha

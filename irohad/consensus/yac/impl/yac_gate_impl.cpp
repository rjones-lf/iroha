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

#include "consensus/yac/impl/yac_gate_impl.hpp"

namespace iroha {
  namespace consensus {
    namespace yac {

      YacGateImpl::YacGateImpl(
          std::shared_ptr<HashGate> hash_gate,
          std::shared_ptr<YacPeerOrderer> orderer,
          std::shared_ptr<YacHashProvider> hash_provider,
          std::shared_ptr<simulator::BlockCreator> block_creator,
          std::shared_ptr<network::BlockLoader> block_loader)
          : hash_gate_(std::move(hash_gate)),
            orderer_(std::move(orderer)),
            hash_provider_(std::move(hash_provider)),
            block_creator_(std::move(block_creator)),
            block_loader_(std::move(block_loader)) {
        log_ = logger::log("YacGate");
        block_creator_->on_block().subscribe([this](auto block) {
          this->vote(block);
        });
      };

      void YacGateImpl::vote(model::Block block) {
        auto hash = hash_provider_->makeHash(block);
        log_->info("vote for block ({}, {})",
                   hash.proposal_hash,
                   hash.block_hash);
        auto order = orderer_->getOrdering(hash);
        if (not order.has_value()) {
          log_->error("ordering doesn't provide peers => pass round");
          return;
        }
        current_block_ = std::make_pair(hash, block);
        hash_gate_->vote(hash, order.value());
      };

      rxcpp::observable<model::Block> YacGateImpl::on_commit() {
        return hash_gate_->on_commit().flat_map([this](auto commit_message) {
          return rxcpp::observable<>::create<model::Block>(
              [this, commit_message](auto subscriber) {
                const auto hash = commit_message.votes.at(0).hash;
                if (hash == current_block_.first) {
                  current_block_.second.sigs.clear();
                  for (auto &&vote : commit_message.votes) {
                    current_block_.second.sigs.push_back(
                        std::move(vote.signature));
                  }
                  log_->info("consensus: commit top block");
                  subscriber.on_next(current_block_.second);
                  subscriber.on_completed();
                  return;
                }
                const auto model_hash = hash_provider_->toModelHash(hash);
                for (const auto &vote : commit_message.votes) {
                  auto block = block_loader_->retrieveBlock(
                      vote.signature.pubkey, model_hash);
                  if (block.has_value()) {
                    subscriber.on_next(block.value());
                    subscriber.on_completed();
                    return;
                  }
                }
                subscriber.on_completed();
              });
        });
      };
    }  // namespace yac
  }    // namespace consensus
}  // namespace iroha

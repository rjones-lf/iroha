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
#include "backend/protobuf/block.hpp"
#include "backend/protobuf/from_old_model.hpp"
#include "builders/protobuf/common_objects/proto_signature_builder.hpp"
#include "consensus/yac/cluster_order.hpp"
#include "consensus/yac/messages.hpp"
#include "consensus/yac/storage/yac_common.hpp"
#include "consensus/yac/yac_hash_provider.hpp"
#include "consensus/yac/yac_peer_orderer.hpp"
#include "cryptography/public_key.hpp"
#include "interfaces/common_objects/signature.hpp"
#include "network/block_loader.hpp"
#include "simulator/block_creator.hpp"
#include "utils/polymorphic_wrapper.hpp"

namespace iroha {
  namespace consensus {
    namespace yac {

      YacGateImpl::YacGateImpl(
          std::shared_ptr<HashGate> hash_gate,
          std::shared_ptr<YacPeerOrderer> orderer,
          std::shared_ptr<YacHashProvider> hash_provider,
          std::shared_ptr<simulator::BlockCreator> block_creator,
          std::shared_ptr<network::BlockLoader> block_loader,
          uint64_t delay)
          : hash_gate_(std::move(hash_gate)),
            orderer_(std::move(orderer)),
            hash_provider_(std::move(hash_provider)),
            block_creator_(std::move(block_creator)),
            block_loader_(std::move(block_loader)),
            delay_(delay) {
        log_ = logger::log("YacGate");
        block_creator_->on_block().subscribe(
            [this](auto block) { this->vote(*block); });
      }

      void YacGateImpl::vote(const shared_model::interface::Block &block) {
        std::unique_ptr<model::Block> bl(block.makeOldModel());
        auto hash = hash_provider_->makeHash(*bl);
        log_->info(
            "vote for block ({}, {})", hash.proposal_hash, block.hash().toString());
        auto order = orderer_->getOrdering(hash);
        if (not order) {
          log_->error("ordering doesn't provide peers => pass round");
          return;
        }
        current_block_ = std::make_pair(
            hash,
            clone(block));
        hash_gate_->vote(hash, *order);
      }

      rxcpp::observable<std::shared_ptr<shared_model::interface::Block>>
      YacGateImpl::on_commit() {
        return hash_gate_->on_commit().flat_map([this](auto commit_message) {
          // map commit to block if it is present or loaded from other peer
          return rxcpp::observable<>::create<
              std::shared_ptr<shared_model::interface::Block>>(
              [this, commit_message](auto subscriber) {
                const auto hash = getHash(commit_message.votes);
                if (not hash) {
                  log_->info("Invalid commit message, hashes are different");
                  subscriber.on_completed();
                  return;
                }
                // if node has voted for the committed block
                if (hash == current_block_.first) {
                  // append signatures of other nodes
                  this->copySignatures(commit_message);
                  log_->info("consensus: commit top block: height {}, hash {}",
                             current_block_.second->height(),
                             current_block_.second->hash().hex());
                  subscriber.on_next(current_block_.second);
                  subscriber.on_completed();
                  return;
                }
                // node has voted for another block - load committed block
                const auto model_hash =
                    hash_provider_->toModelHash(hash.value());
                // iterate over peers who voted for the committed block
                rxcpp::observable<>::iterate(commit_message.votes)
                    // allow other peers to apply commit
                    .delay(std::chrono::milliseconds(delay_))
                    .flat_map([this, model_hash](auto vote) {
                      // map vote to block if it can be loaded
                      return rxcpp::observable<>::create<
                          std::shared_ptr<shared_model::interface::Block>>(
                          [this, model_hash, vote](auto subscriber) {
                            auto block = block_loader_->retrieveBlock(
                                shared_model::crypto::PublicKey(
                                    {vote.signature.pubkey.begin(),
                                     vote.signature.pubkey.end()}),
                                shared_model::crypto::Hash(
                                    {model_hash.begin(), model_hash.end()}));
                            // if load is successful
                            if (block) {
                              subscriber.on_next(block.value());
                            }
                            subscriber.on_completed();
                          });
                    })
                    // need only the first
                    .first()
                    .subscribe(
                        // if load is successful from at least one node
                        [subscriber](auto block) {
                          subscriber.on_next(block);
                          subscriber.on_completed();
                        },
                        // if load has failed, no peers provided the block
                        [this, subscriber](std::exception_ptr) {
                          log_->error("Cannot load committed block");
                          subscriber.on_completed();
                        });
              });
        });
      }

      void YacGateImpl::copySignatures(const CommitMessage &commit) {
        current_block_.second->clearSignatures();
        for (const auto &vote : commit.votes) {
          auto sig = vote.hash.block_signature;
          auto tmp =
              shared_model::proto::SignatureBuilder()
                  .signedData(shared_model::interface::Signature::SignedType(
                      sig.signature.to_string()))
                  .publicKey(
                      shared_model::crypto::PublicKey(sig.pubkey.to_string()))
                  .build();
          auto wrap = shared_model::detail::makePolymorphic<
              shared_model::proto::Signature>(tmp.getTransport());
          current_block_.second->addSignature(wrap);
        }
      }
    }  // namespace yac
  }    // namespace consensus
}  // namespace iroha

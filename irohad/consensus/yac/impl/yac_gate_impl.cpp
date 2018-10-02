/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "consensus/yac/impl/yac_gate_impl.hpp"

#include "common/visitor.hpp"
#include "consensus/yac/cluster_order.hpp"
#include "consensus/yac/messages.hpp"
#include "consensus/yac/storage/yac_common.hpp"
#include "consensus/yac/yac_hash_provider.hpp"
#include "consensus/yac/yac_peer_orderer.hpp"
#include "cryptography/public_key.hpp"
#include "interfaces/common_objects/signature.hpp"
#include "network/block_loader.hpp"
#include "simulator/block_creator.hpp"

namespace iroha {
  namespace consensus {
    namespace yac {

      YacGateImpl::YacGateImpl(
          std::shared_ptr<HashGate> hash_gate,
          std::shared_ptr<YacPeerOrderer> orderer,
          std::shared_ptr<YacHashProvider> hash_provider,
          std::shared_ptr<simulator::BlockCreator> block_creator,
          std::shared_ptr<network::BlockLoader> block_loader,
          std::shared_ptr<consensus::ConsensusResultCache>
              consensus_result_cache)
          : hash_gate_(std::move(hash_gate)),
            orderer_(std::move(orderer)),
            hash_provider_(std::move(hash_provider)),
            block_creator_(std::move(block_creator)),
            block_loader_(std::move(block_loader)),
            consensus_result_cache_(std::move(consensus_result_cache)),
            log_(logger::log("YacGate")) {
        block_creator_->on_block().subscribe(
            [this](auto block) { this->vote(block); });
      }

      void YacGateImpl::vote(std::shared_ptr<shared_model::interface::Block> block) {
        auto hash = hash_provider_->makeHash(*block);
        log_->info("vote for block ({}, {})",
                   hash.vote_hashes.proposal_hash,
                   block->hash().toString());
        auto order = orderer_->getOrdering(hash);
        if (not order) {
          log_->error("ordering doesn't provide peers => pass round");
          return;
        }
        current_block_ = std::make_pair(hash, block);
        hash_gate_->vote(hash, *order);

        // insert the block we voted for to the consensus cache
        consensus_result_cache_->insert(block);
      }

      rxcpp::observable<std::shared_ptr<shared_model::interface::Block>>
      YacGateImpl::on_commit() {
        return hash_gate_->onOutcome().flat_map([this](auto message) {
          // TODO 10.06.2018 andrei: IR-497 Work on reject case
          auto commit_message = boost::get<CommitMessage>(message);
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
                    .flat_map([this, model_hash](auto vote) {
                      // map vote to block if it can be loaded
                      return rxcpp::observable<>::create<
                          std::shared_ptr<shared_model::interface::Block>>(
                          [this, model_hash, vote](auto subscriber) {
                            auto block = block_loader_->retrieveBlock(
                                vote.signature->publicKey(),
                                shared_model::crypto::Hash(model_hash));
                            // if load is successful
                            if (block) {
                              // update the cache with block consensus voted for
                              consensus_result_cache_->insert(*block);
                              subscriber.on_next(*block);
                            } else {
                              log_->error(
                                  "Could not get block from block loader");
                            }
                            subscriber.on_completed();
                          });
                    })
                    // need only the first
                    .first()
                    .retry()
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
        for (const auto &vote : commit.votes) {
          auto sig = vote.hash.block_signature;
          current_block_.second->addSignature(sig->signedData(),
                                              sig->publicKey());
        }
      }
    }  // namespace yac
  }    // namespace consensus
}  // namespace iroha

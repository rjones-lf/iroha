/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "consensus/yac/impl/yac_gate_impl.hpp"

#include <boost/range/adaptor/transformed.hpp>
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
          std::shared_ptr<consensus::ConsensusResultCache>
              consensus_result_cache)
          : hash_gate_(std::move(hash_gate)),
            orderer_(std::move(orderer)),
            hash_provider_(std::move(hash_provider)),
            block_creator_(std::move(block_creator)),
            consensus_result_cache_(std::move(consensus_result_cache)),
            log_(logger::log("YacGate")) {
        block_creator_->on_block().subscribe([this](auto block) {
          // TODO(@l4l) 24/09/18 IR-1717
          // update BlockCreator iface according to YacGate
          this->vote({std::shared_ptr<shared_model::interface::Proposal>()},
                     {block},
                     {0, 0});
        });
      }

      void YacGateImpl::vote(
          boost::optional<std::shared_ptr<shared_model::interface::Proposal>>
              proposal,
          boost::optional<std::shared_ptr<shared_model::interface::Block>>
              block,
          Round round) {
        // TODO IR-1717: uncomment
        bool is_none = /*not proposal or */ not block;
        if (is_none) {
          current_block_ = boost::none;
          current_hash_ = {};
          current_hash_.vote_round = round;
          log_->debug("Agreed on nothing to commit");
        } else {
          current_block_ = block.value();
          // TODO IR-1717: uncomment
          current_hash_ = hash_provider_->makeHash(
              *current_block_.value() /*, *proposal, round*/);
          log_->info("vote for (proposal: {}, block: {})",
                     current_hash_.vote_hashes.proposal_hash,
                     current_hash_.vote_hashes.block_hash);
        }

        auto order = orderer_->getOrdering(current_hash_);
        if (not order) {
          log_->error("ordering doesn't provide peers => pass round");
          return;
        }

        hash_gate_->vote(current_hash_, *order);

        // insert the block we voted for to the consensus cache
        if (not is_none) {
          consensus_result_cache_->insert(block.value());
        }
      }

      rxcpp::observable<YacGateImpl::GateObject> YacGateImpl::onOutcome() {
        return hash_gate_->onOutcome().flat_map([this](auto message) {
          return visit_in_place(message,
                                [this](const CommitMessage &msg) {
                                  return this->handleCommit(msg);
                                },
                                [this](const RejectMessage &msg) {
                                  return this->handleReject(msg);
                                });
        });
      }

      void YacGateImpl::copySignatures(const CommitMessage &commit) {
        for (const auto &vote : commit.votes) {
          auto sig = vote.hash.block_signature;
          current_block_.value()->addSignature(sig->signedData(),
                                               sig->publicKey());
        }
      }

      rxcpp::observable<YacGateImpl::GateObject> YacGateImpl::handleCommit(
          const CommitMessage &msg) {
        const auto hash = getHash(msg.votes).value();
        if (hash.vote_hashes.proposal_hash.empty()) {
          // if consensus agreed on nothing for commit
          log_->debug("Consensus skipped round, voted for nothing");
          return rxcpp::observable<>::just<GateObject>(
              AgreementOnNone{current_hash_.vote_round});
        } else if (hash == current_hash_) {
          // if node has voted for the committed block
          // append signatures of other nodes
          this->copySignatures(msg);
          auto &block = current_block_.value();
          log_->info("consensus: commit top block: height {}, hash {}",
                     block->height(),
                     block->hash().hex());
          return rxcpp::observable<>::just<GateObject>(
              PairValid{block, current_hash_.vote_round});
        }
        log_->info("Voted for another block, waiting for sync");
        auto public_keys = boost::copy_range<
            shared_model::interface::types::PublicKeyCollectionType>(
            msg.votes | boost::adaptors::transformed([](auto &vote) {
              return vote.signature->publicKey();
            }));
        auto model_hash = hash_provider_->toModelHash(hash);
        return rxcpp::observable<>::just<GateObject>(
            VoteOther{std::move(public_keys),
                      std::move(model_hash),
                      current_hash_.vote_round});
      }

      rxcpp::observable<YacGateImpl::GateObject> YacGateImpl::handleReject(
          const RejectMessage &msg) {
        const auto hash = getHash(msg.votes);
        if (not hash) {
          log_->info("Proposal reject since all hashes are different");
          return rxcpp::observable<>::just<GateObject>(
              ProposalReject{current_hash_.vote_round});
        }
        log_->info("Block reject since proposal hashes match");
        return rxcpp::observable<>::just<GateObject>(
            BlockReject{current_hash_.vote_round});
      }
    }  // namespace yac
  }    // namespace consensus
}  // namespace iroha

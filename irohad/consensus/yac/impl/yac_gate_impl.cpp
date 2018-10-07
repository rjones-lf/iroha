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
        bool is_none = not proposal or not block;
        if (is_none) {
          current_block_ = nullptr;
          current_hash_ = {};
          log_->debug("Agreed on nothing to commit");
        } else {
          current_block_ = std::move(block.value());
          current_hash_ =
              hash_provider_->makeHash(*current_block_ /*, *proposal, round*/);
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
        if (is_none) {
          consensus_result_cache_->insert(block.value());
        }
      }

      rxcpp::observable<YacGateImpl::GateObject> YacGateImpl::onOutcome() {
        return hash_gate_->onOutcome().flat_map([this](auto message) {
          return visit_in_place(
              message,
              [this](const CommitMessage &msg) { return handleCommit(msg); },
              [this](const RejectMessage &msg) { return handleReject(msg); });
        });
      }

      void YacGateImpl::copySignatures(const CommitMessage &commit) {
        for (const auto &vote : commit.votes) {
          auto sig = vote.hash.block_signature;
          current_block_->addSignature(sig->signedData(), sig->publicKey());
        }
      }

      rxcpp::observable<YacGateImpl::GateObject> YacGateImpl::handleCommit(
          const CommitMessage &msg) {
        const auto hash = getHash(msg.votes).value();
        if (hash.vote_hashes.proposal_hash.empty()) {
          // if consensus agreed on nothing for commit
          log_->debug("Consensus skipped round, voted for nothing");
          return rxcpp::observable<>::just<GateObject>(
              network::AgreementOnNone{});
        } else if (hash == current_hash_) {
          // if node has voted for the committed block
          // append signatures of other nodes
          this->copySignatures(msg);
          log_->info("consensus: commit top block: height {}, hash {}",
                     current_block_->height(),
                     current_block_->hash().hex());
          return rxcpp::observable<>::just<GateObject>(
              network::PairValid{current_block_});
        } else {
          log_->info("Voted for another block, waiting for sync");
          return rxcpp::observable<>::just<GateObject>(
              network::VoteOther{current_block_});
        }
        return rxcpp::observable<>::empty<GateObject>();
      }

      rxcpp::observable<YacGateImpl::GateObject> YacGateImpl::handleReject(
          const RejectMessage &msg) {
        const auto hash = getHash(msg.votes);
        if (not hash) {
          log_->info("Proposal reject since all hashes are different");
          return rxcpp::observable<>::just<GateObject>(
              network::ProposalReject{});
        }
        log_->info("Block reject since proposal hashes match");
        return rxcpp::observable<>::just<GateObject>(
            network::BlockReject{current_block_});
      }
    }  // namespace yac
  }    // namespace consensus
}  // namespace iroha

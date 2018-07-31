/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "consensus/yac/impl/yac_proposal_gate_impl.hpp"

#include "common/visitor.hpp"
#include "consensus/yac/cluster_order.hpp"
#include "consensus/yac/messages.hpp"
#include "interfaces/iroha_internal/proposal.hpp"

using namespace iroha;
using namespace iroha::network;
using namespace iroha::consensus::yac;

YacProposalGateImpl::YacProposalGateImpl(
    std::shared_ptr<HashGate> hash_gate,
    std::shared_ptr<YacPeerOrderer> orderer,
    std::shared_ptr<YacProposalHashProvider> hash_provider)
    : hash_gate_(std::move(hash_gate)),
      orderer_(std::move(orderer)),
      hash_provider_(std::move(hash_provider)),
      log_(logger::log("YacProposalGateImpl")) {}

expected::Result<void, std::string> YacProposalGateImpl::vote(
    ProposalVote vote) {
  auto hash = hash_provider_->makeHash(vote);
  log_->info("vote for proposal ({}, {}, {})",
             vote.proposal ? vote.proposal.value()->hash().toString() : "''",
             vote.round.first,
             vote.round.second);
  auto order = orderer_->getOrdering(hash);
  if (not order) {
    return expected::makeError("Orderer doesn't provide peers");
  }
  last_voted_proposal_.emplace(hash, std::move(vote));
  hash_gate_->vote(hash, *order);
  return {};
}

rxcpp::observable<ProposalOutcomeType> YacProposalGateImpl::outcomes() {
  return hash_gate_->onOutcome().map([this](auto outcome) {
    auto hash = visit_in_place(
        outcome, [](auto outcome) { return outcome.votes.at(0).hash; });
    auto proposal_info = hash_provider_->makeProposalInfo(hash);
    auto result = visit_in_place(
        outcome,
        [&](CommitMessage commit) -> ProposalOutcomeType {
          bool has_proposal =
              last_voted_proposal_ | [&](const auto &last_voted_proposal) {
                return hash == last_voted_proposal.first;
              };
          if (not has_proposal) {
            // TODO andrei 30.07.2018 IR-1435 proposal consensus cache
            throw std::logic_error("Not implemented yet");
          }
          std::shared_ptr<shared_model::interface::Proposal> proposal(
              std::move(last_voted_proposal_->second.proposal).value());
          return ProposalCommit{proposal, proposal_info.round};
        },
        [&](RejectMessage reject) -> ProposalOutcomeType {
          return ProposalReject{proposal_info.round};
        });
    last_voted_proposal_.reset();
    return result;
  });
}

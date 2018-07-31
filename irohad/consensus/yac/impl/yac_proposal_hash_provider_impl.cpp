/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "consensus/yac/impl/yac_proposal_hash_provider_impl.hpp"

#include <sstream>

#include "interfaces/iroha_internal/proposal.hpp"

using namespace iroha::network;
using namespace iroha::consensus::yac;

YacHash YacProposalHashProviderImpl::makeHash(const ProposalVote &vote) const {
  auto round = std::to_string(vote.round.first) + " "
      + std::to_string(vote.round.second);
  auto proposal_vote = vote.proposal ? vote.proposal.value()->hash().hex() : "";
  return YacHash(round, proposal_vote);
}

YacProposalHashProvider::ProposalInfo
YacProposalHashProviderImpl::makeProposalInfo(const YacHash &hash) const {
  decltype(std::declval<ProposalInfo>().hash) proposal_hash;
  if (not hash.block_hash.empty()) {
    proposal_hash.emplace(
        shared_model::crypto::Blob::fromHexString(hash.block_hash));
  }
  std::istringstream iss(hash.proposal_hash);
  decltype(std::declval<ProposalInfo>().round.first) round;
  iss >> round;
  decltype(std::declval<ProposalInfo>().round.second) reject;
  iss >> reject;
  return {proposal_hash, {round, reject}};
}

/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef INTEGRATION_FRAMEWORK_FAKE_PEER_PROPOSAL_STORAGE_HPP_
#define INTEGRATION_FRAMEWORK_FAKE_PEER_PROPOSAL_STORAGE_HPP_

#include <map>

#include "backend/protobuf/proposal.hpp"
#include "consensus/round.hpp"
#include "framework/integration_framework/fake_peer/types.hpp"

namespace integration_framework {
  namespace fake_peer {

    class ProposalStorage final {
     public:
      using Round = iroha::consensus::Round;
      using Proposal = shared_model::proto::Proposal;
      using DefaultProvider = std::function<OrderingProposalRequestResult(
          const OrderingProposalRequest &)>;

      ProposalStorage();

      OrderingProposalRequestResult getProposal(
          const OrderingProposalRequest &round) const;

      ProposalStorage &storeProposal(const Round &round,
                                     std::shared_ptr<Proposal> proposal);

      ProposalStorage &setDefaultProvider(DefaultProvider default_provider);

     private:
      DefaultProvider default_provider_;
      std::map<Round, std::shared_ptr<Proposal>> proposals_map_;
    };

  }  // namespace fake_peer
}  // namespace integration_framework

#endif /* INTEGRATION_FRAMEWORK_FAKE_PEER_PROPOSAL_STORAGE_HPP_ */

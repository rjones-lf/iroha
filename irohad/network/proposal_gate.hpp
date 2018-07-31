/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_PROPOSAL_GATE_HPP
#define IROHA_PROPOSAL_GATE_HPP

#include <boost/variant.hpp>
#include <rxcpp/rx.hpp>
#include "common/result.hpp"
#include "ordering/on_demand_os_transport.hpp"

namespace iroha {
  namespace network {

    /**
     * Represents proposal vote of a peer
     * Peer can vote either for no proposal received,
     * reprensented by boost::none, or for a value
     */
    struct ProposalVote {
      boost::optional<ordering::transport::OdOsNotification::ProposalType>
          proposal;

      ordering::transport::RoundType round;
    };

    /**
     * Commit message represents agreement on a particular proposal
     * Similarly, proposal could be empty (boost::none), or a value
     */
    struct ProposalCommit {
      boost::optional<std::shared_ptr<shared_model::interface::Proposal>>
          proposal;

      ordering::transport::RoundType round;
    };

    /**
     * Reject message represent lack of agreement on a particular proposal
     * Since there is no committed value, it only contains round number for
     * identification
     */
    struct ProposalReject {
      ordering::transport::RoundType round;
    };

    /// Consensus outcome can be either agreed committed value, or reject
    using ProposalOutcomeType = boost::variant<ProposalCommit, ProposalReject>;

    /**
     * Allows to vote for proposals and receive consensus outcomes
     */
    class ProposalGate {
     public:
      /**
       * Vote for proposal
       * @param vote - structure with optional proposal and round number
       * @return void on success, or error message represented by std::string
       */
      virtual expected::Result<void, std::string> vote(ProposalVote vote) = 0;

      /**
       * Receive consensus outcomes for rounds
       * @return - observable with outcomes, commits and rejects
       */
      virtual rxcpp::observable<ProposalOutcomeType> outcomes() = 0;

      virtual ~ProposalGate() = default;
    };

  }  // namespace network
}  // namespace iroha

#endif  // IROHA_PROPOSAL_GATE_HPP

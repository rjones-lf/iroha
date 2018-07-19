/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_PROTO_PROPOSAL_FACTORY_HPP
#define IROHA_PROTO_PROPOSAL_FACTORY_HPP

#include "interfaces/iroha_internal/proposal_factory.hpp"

#include "backend/protobuf/proposal.hpp"
#include "proposal.pb.h"

namespace shared_model {
  namespace proto {

    template <typename Validator>
    class ProtoProposalFactory : public interface::ProposalFactory {
     public:
      FactoryResult<std::unique_ptr<interface::Proposal>> createProposal(
          interface::types::HeightType height,
          interface::types::TimestampType created_time,
          const interface::types::TransactionsCollectionType &transactions)
          override {
        iroha::protocol::Proposal proposal;

        proposal.set_height(height);
        proposal.set_created_time(created_time);

        for (const auto &tx : transactions) {
          *proposal.add_transactions() =
              static_cast<const shared_model::proto::Transaction &>(tx)
                  .getTransport();
        }

        auto proto_proposal = std::make_unique<Proposal>(std::move(proposal));

        auto errors = validator_.validate(*proto_proposal);

        if (errors) {
          return iroha::expected::makeError(errors.reason());
        }

        return iroha::expected::makeValue<std::unique_ptr<interface::Proposal>>(
            std::move(proto_proposal));
      }

     private:
      Validator validator_;
    };
  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_PROTO_PROPOSAL_FACTORY_HPP

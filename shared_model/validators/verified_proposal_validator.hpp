/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_VERIFIED_PROPOSAL_VALIDATOR_HPP
#define IROHA_VERIFIED_PROPOSAL_VALIDATOR_HPP

#include "interfaces/iroha_internal/verified_proposal.hpp"
#include "validators/container_fields/container_validator.hpp"
#include "validators/container_fields/transactions_validator.hpp"

namespace shared_model {
  namespace validation {

    template <typename FieldValidator, typename TransactionValidator>
    class VerifiedProposalValidator
        : public ContainerValidator<
              interface::VerifiedProposal,
              FieldValidator,
              TransactionsValidator<TransactionValidator>> {
     public:
      /**
       * Applies validation on block
       * @param block
       * @return Answer containing found error if any
       */
      Answer validate(
          const interface::VerifiedProposal &verified_proposal) const {
        return ContainerValidator<interface::VerifiedProposal,
                                  FieldValidator,
                                  TransactionsValidator<TransactionValidator>>::
            validate(verified_proposal, "verified_proposal");
      }
    };

  }  // namespace validation
}  // namespace shared_model

#endif  // IROHA_VERIFIED_PROPOSAL_VALIDATOR_HPP

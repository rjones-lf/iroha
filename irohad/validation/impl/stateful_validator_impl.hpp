/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_STATEFUL_VALIDATIOR_IMPL_HPP
#define IROHA_STATEFUL_VALIDATIOR_IMPL_HPP

#include "interfaces/iroha_internal/unsafe_proposal_factory.hpp"
#include "validation/stateful_validator.hpp"

#include "logger/logger.hpp"

namespace iroha {
  namespace validation {

    /**
     * Interface for performing stateful validation
     */
    class StatefulValidatorImpl : public StatefulValidator {
     public:
      explicit StatefulValidatorImpl(
          std::unique_ptr<shared_model::interface::UnsafeProposalFactory>
              factory);

      VerifiedProposalAndErrors validate(
          const shared_model::interface::Proposal &proposal,
          ametsuchi::TemporaryWsv &temporaryWsv) override;

      std::unique_ptr<shared_model::interface::UnsafeProposalFactory> factory_;
      logger::Logger log_;
    };

  }  // namespace validation
}  // namespace iroha

#endif  // IROHA_STATEFUL_VALIDATION_IMPL_HPP

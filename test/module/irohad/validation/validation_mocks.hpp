/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_VALIDATION_MOCKS_HPP
#define IROHA_VALIDATION_MOCKS_HPP

#include <gmock/gmock.h>

#include "interfaces/common_objects/types.hpp"
#include "interfaces/iroha_internal/proposal.hpp"
#include "validation/chain_validator.hpp"
#include "validation/stateful_validator.hpp"

namespace iroha {
  namespace validation {
    class MockStatefulValidator : public validation::StatefulValidator {
     public:
      MOCK_METHOD2(validate,
                   std::unique_ptr<VerifiedProposalAndErrors>(
                       const shared_model::interface::Proposal &,
                       ametsuchi::TemporaryWsv &));
    };

    class MockChainValidator : public ChainValidator {
     public:
      MOCK_CONST_METHOD2(
          validateAndApply,
          bool(rxcpp::observable<
                   std::shared_ptr<shared_model::interface::Block>>,
               ametsuchi::MutableStorage &));
    };
  }  // namespace validation
}  // namespace iroha

#endif  // IROHA_VALIDATION_MOCKS_HPP

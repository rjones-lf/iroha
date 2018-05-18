/*
Copyright Soramitsu Co., Ltd. 2016 All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#ifndef IROHA_VALIDATION_STATEFUL_VALIDATOR_HPP
#define IROHA_VALIDATION_STATEFUL_VALIDATOR_HPP

#include "ametsuchi/temporary_wsv.hpp"
#include "interfaces/iroha_internal/proposal.hpp"
#include "interfaces/iroha_internal/verified_proposal.hpp"

namespace iroha {
  namespace validation {

    /**
     * Interface for performing stateful validation
     */
    class StatefulValidator {
     public:
      virtual ~StatefulValidator() = default;

      /**
       * Function perform stateful validation on proposal
       * and return proposal with valid transactions
       * @param proposal - proposal for validation
       * @param wsv  - temporary wsv for validation,
       * this wsv not affected on ledger,
       * all changes after removing wsv will be ignored
       * @return proposal with valid transactions
       */
      virtual std::shared_ptr<shared_model::interface::VerifiedProposal> validate(
          const shared_model::interface::Proposal &proposal,
          ametsuchi::TemporaryWsv &temporaryWsv) = 0;
    };
  }  // namespace validation
}  // namespace iroha
#endif  // IROHA_VALIDATION_STATELESS_VALIDATOR_HPP

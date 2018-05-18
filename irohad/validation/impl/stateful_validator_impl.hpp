/**
 * Copyright Soramitsu Co., Ltd. 2017 All Rights Reserved.
 * http://soramitsu.co.jp
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef IROHA_STATEFUL_VALIDATIOR_IMPL_HPP
#define IROHA_STATEFUL_VALIDATIOR_IMPL_HPP

#include "validation/stateful_validator.hpp"

#include "logger/logger.hpp"

namespace iroha {
  namespace validation {

    /**
     * Interface for performing stateful validation
     */
    class StatefulValidatorImpl : public StatefulValidator {
     public:
      StatefulValidatorImpl();

      /**
       * Function perform stateful validation on proposal
       * and return proposal with valid transactions
       * @param proposal - proposal for validation
       * @param wsv  - temporary wsv for validation,
       * this wsv not affected on ledger,
       * all changes after removing wsv will be ignored
       * @return proposal with valid transactions
       */
      std::shared_ptr<shared_model::interface::VerifiedProposal> validate(
          const shared_model::interface::Proposal &proposal,
          ametsuchi::TemporaryWsv &temporaryWsv) override;

     private:
      /**
       * Checks if public keys of signatures are present in vector of pubkeys
       * @param signatures - collection of signatures
       * @param public_keys - collection of public keys
       * @return true, if all public keys of signatures are present in vector of
       * pubkeys
       */
      bool signaturesSubset(
          const shared_model::interface::types::SignatureRangeType &signatures,
          const std::vector<shared_model::crypto::PublicKey> &public_keys);

      logger::Logger log_;
    };
  }  // namespace validation
}  // namespace iroha

#endif  // IROHA_STATEFUL_VALIDATION_IMPL_HPP

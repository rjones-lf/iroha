/**
 * Copyright Soramitsu Co., Ltd. 2018 All Rights Reserved.
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

#ifndef IROHA_PROPOSAL_VALIDATOR_HPP
#define IROHA_PROPOSAL_VALIDATOR_HPP

#include <regex>
#include "datetime/time.hpp"
#include "interfaces/common_objects/types.hpp"
#include "interfaces/iroha_internal/proposal.hpp"
#include "validators/answer.hpp"

// TODO 22/01/2018 x3medima17: write stateless validator IR-836

namespace shared_model {
  namespace validation {

    /**
     * Class that validates proposal
     */
    class ProposalValidator {
     private:
      void validateTransaction(
          ReasonsGroupType &reason,
          const interface::Transaction &transaction) const {
        // TODO 22/01/2018 x3medima17: add stateless validator IR-837
      }

      void validateHeight(ReasonsGroupType &reason,
                          const interface::types::HeightType &height) const {
        // TODO 22/01/2018 x3medima17: add stateless validator IR-837
      }

     public:
      /**
       * Applies validation on proposal
       * @param proposal
       * @return Answer containing found error if any
       */
      Answer validate(const interface::Proposal &prop) const {
        Answer answer;
        // TODO 22/01/2018 x3medima17: add stateless validator IR-837
        ReasonsGroupType reason;
        reason.first = "Proposal";

        validateHeight(reason, prop.height());
        for (const auto &tx : prop.transactions()) {
          validateTransaction(reason, *tx);
        }
        if (not reason.second.empty()) {
          answer.addReason(std::move(reason));
        }

        return answer;
      }

      Answer answer_;
    };

  }  // namespace validation
}  // namespace shared_model

#endif  // IROHA_PROPOSAL_VALIDATOR_HPP

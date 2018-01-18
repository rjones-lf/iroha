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

#ifndef IROHA_PROPOSAL_VALIDATOR_HPP_HPP
#define IROHA_PROPOSAL_VALIDATOR_HPP_HPP

#include <boost/format.hpp>
#include <boost/variant/static_visitor.hpp>
#include <regex>
#include "datetime/time.hpp"
#include "interfaces/common_objects/types.hpp"
#include "utils/polymorphic_wrapper.hpp"
#include "validators/answer.hpp"

namespace shared_model {
  namespace validation {

    /**
     * Class that validates proposal
     */
    class ProposalValidator {
    private:
      /**
       * Visitor used by commands validator to validate fields from proposal
       */
      class ProposalValidatorVisitor
              : public boost::static_visitor<ReasonsGroupType> {
      public:
        ReasonsGroupType operator()(
                const detail::PolymorphicWrapper<interface::Proposal> &prop)
        const {
          ReasonsGroupType reason;
          reason.first = "Proposal";

          validateHeight(reason, prop->height());
          for(const auto& tx: prop->transactions()) {
            validateTransaction(reason, tx);
          }
          return reason;
        }

      private:
        void validateTransaction(
                ReasonsGroupType &reason,
                const interface::Transaction &transaction) const {
          //TODO write transaction validator
        }

        void validateHeight(
                ReasonsGroupType &reason,
                const interface::types::HeightType &height) const {

          if (height < 0) {
            reason.second.push_back("Wrongly formed asset_id");
          }
        }

      };

    public:
      /**
       * Applies validation on proposal
       * @param proposal
       * @return Answer containing found error if any
       */
      Answer validate(
              detail::PolymorphicWrapper<interface::Proposal> prop) const {
        Answer answer;
        std::string prop_reason_name = "Proposal";
        ReasonsGroupType prop_reason(prop_reason_name, GroupedReasons());

        //TODO Write validator

        return answer;
      }

      Answer answer_;
    };

  }  // namespace validation
}  // namespace shared_model


#endif //IROHA_PROPOSAL_VALIDATOR_HPP_HPP

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

#ifndef IROHA_SHARED_MODEL_SIGNABLE_VALIDATOR_HPP
#define IROHA_SHARED_MODEL_SIGNABLE_VALIDATOR_HPP

#include "validators/query_validator.hpp"
#include "validators/transaction_validator.hpp"

namespace shared_model {
  namespace validation {

    template <typename ModelValidator, typename Model, typename FieldValidator>
    class SignableModelValidator : public ModelValidator {
      // using ModelValidator::ModelValidator;
     public:
      Answer validate(const Model &model) const {
        auto answer = ModelValidator::validate(model);
        std::string reason_name = "Signature";
        ReasonsGroupType reason(reason_name, GroupedReasons());
        FieldValidator().validateSignatures(
            reason, model.signatures(), model.payload());
        if (not reason.second.empty()) {
          answer.addReason(std::move(reason));
        }
        return answer;
      }
    };
  }  // namespace validation
}  // namespace shared_model
#endif  // IROHA_SHARED_MODEL_SIGNABLE_VALIDATOR_HPP

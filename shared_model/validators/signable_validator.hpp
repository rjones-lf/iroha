/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_SHARED_MODEL_SIGNABLE_VALIDATOR_HPP
#define IROHA_SHARED_MODEL_SIGNABLE_VALIDATOR_HPP

#include "validators/query_validator.hpp"
#include "validators/transaction_validator.hpp"

namespace shared_model {
  namespace validation {

    template <typename ModelValidator, typename Model, typename FieldValidator>
    class SignableModelValidator : public ModelValidator {
     private:
      template <typename Validator>
      Answer validateImpl(const Model &model, Validator &&validator) const {
        auto answer = std::forward<Validator>(validator)(model);
        std::string reason_name = "Signature";
        ReasonsGroupType reason(reason_name, GroupedReasons());
        field_validator_.validateSignatures(
            reason, model.signatures(), model.payload());
        if (not reason.second.empty()) {
          answer.addReason(std::move(reason));
        }
        return answer;
      }

     public:
      explicit SignableModelValidator(
          FieldValidator &&validator = FieldValidator())
          : ModelValidator(validator), field_validator_(std::move(validator)) {}

      Answer validate(const Model &model,
                      interface::types::TimestampType current_timestamp) const {
        return validateImpl(
            model, [this, current_timestamp](const auto &model) {
              return ModelValidator::validate(model, current_timestamp);
            });
      }

      Answer validate(const Model &model) const {
        return validateImpl(model, [this](const auto &model) {
          return ModelValidator::validate(model);
        });
      }

     private:
      FieldValidator field_validator_;
    };
  }  // namespace validation
}  // namespace shared_model
#endif  // IROHA_SHARED_MODEL_SIGNABLE_VALIDATOR_HPP

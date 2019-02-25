/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_SHARED_MODEL_SIGNABLE_VALIDATOR_HPP
#define IROHA_SHARED_MODEL_SIGNABLE_VALIDATOR_HPP

#include "validators/answer.hpp"

namespace shared_model {
  namespace validation {

    template <typename ModelValidator,
              typename Model,
              typename FieldValidator,
              bool SignatureRequired = true>
    class SignableModelValidator : public ModelValidator {
     private:
      template <typename Validator>
      Answer validateImpl(const Model &model, Validator &&validator) const {
        auto answer = std::forward<Validator>(validator)(model);
        std::string reason_name = "Signature";
        ReasonsGroupType reason(reason_name, GroupedReasons());
        if (SignatureRequired or not model.signatures().empty()) {
          field_validator_.validateSignatures(
              reason, model.signatures(), model.payload());
        }
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
        return validateImpl(model, [&, current_timestamp](const Model &m) {
          return ModelValidator::validate(m, current_timestamp);
        });
      }

      Answer validate(const Model &model) const {
        return validateImpl(
            model, [&](const Model &m) { return ModelValidator::validate(m); });
      }

     private:
      FieldValidator field_validator_;
    };
  }  // namespace validation
}  // namespace shared_model
#endif  // IROHA_SHARED_MODEL_SIGNABLE_VALIDATOR_HPP

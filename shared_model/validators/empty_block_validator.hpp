/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_EMPTY_BLOCK_VALIDATOR_HPP
#define IROHA_EMPTY_BLOCK_VALIDATOR_HPP

#include <boost/format.hpp>
#include "datetime/time.hpp"
#include "interfaces/common_objects/types.hpp"
#include "interfaces/iroha_internal/block.hpp"
#include "utils/polymorphic_wrapper.hpp"
#include "validators/answer.hpp"
#include "validators/container_fields/height_validator.hpp"

namespace shared_model {
  namespace validation {

    /**
     * Class that validates empty block
     */
    template <typename FieldValidator>
    class EmptyBlockValidator : public HeightValidator {
     public:
      EmptyBlockValidator(FieldValidator field_validator = FieldValidator())
          : field_validator_(field_validator) {}
      /**
       * Applies validation on block
       * @param block
       * @return Answer containing found error if any
       */
      Answer validate(const interface::EmptyBlock &block) const {
        Answer answer;
        ReasonsGroupType reason;
        reason.first = "EmptyBlock";
        field_validator_.validateCreatedTime(reason, block.createdTime());
        HeightValidator::validateHeight(reason, block.height());
        if (not reason.second.empty()) {
          answer.addReason(std::move(reason));
        }
        return answer;
      }

     private:
      FieldValidator field_validator_;
    };

  }  // namespace validation
}  // namespace shared_model

#endif  // IROHA_EMPTY_BLOCK_VALIDATOR_HPP

/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_EMPTY_BLOCK_VALIDATOR_HPP
#define IROHA_EMPTY_BLOCK_VALIDATOR_HPP

#include "interfaces/iroha_internal/empty_block.hpp"
#include "validators/answer.hpp"

namespace shared_model {
  namespace validation {

    template <typename FieldValidator>
    class EmptyBlockValidator {
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

        field_validator_.validateHeight(reason, block.height());
        field_validator_.validateHash(reason, block.prevHash());

        if (not reason.second.empty()) {
          answer.addReason(std::move(reason));
        }
        return answer;
      }

     protected:
      FieldValidator field_validator_;
    };

  }  // namespace validation
}  // namespace shared_model

#endif  // IROHA_BLOCK_VALIDATOR_HPP

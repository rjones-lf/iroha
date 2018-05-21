/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_ANY_BLOCK_VALIDATOR_HPP
#define IROHA_ANY_BLOCK_VALIDATOR_HPP

#include "common/visitor.hpp"
#include "interfaces/common_objects/types.hpp"
#include "interfaces/iroha_internal/block_variant.hpp"
#include "validators/answer.hpp"

namespace shared_model {
  namespace validation {

    template <typename BlockValidator, typename EmptyBlockValidator>
    class AnyBlockValidator {
     public:
      AnyBlockValidator(
          BlockValidator block_validator = BlockValidator(),
          EmptyBlockValidator empty_block_validator = EmptyBlockValidator())
          : non_empty_block_validator_(block_validator),
            empty_block_validator_(empty_block_validator) {}

      Answer validate(const interface::Block &block) const {
        return non_empty_block_validator_.validate(block);
      }

      Answer validate(const interface::EmptyBlock &empty_block) const {
        return empty_block_validator_.validate(empty_block);
      }

      Answer validate(const interface::BlockVariantType &block_variant) const {
        return iroha::visit_in_place(
            block_variant, [this](auto block) { return validate(*block); });
      }

     private:
      BlockValidator non_empty_block_validator_;
      EmptyBlockValidator empty_block_validator_;
    };
  }  // namespace validation
}  // namespace shared_model

#endif  // IROHA_ANY_BLOCK_VALIDATOR_HPP

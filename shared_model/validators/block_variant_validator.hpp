/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_BLOCK_VARIANT_VALIDATOR_HPP
#define IROHA_BLOCK_VARIANT_VALIDATOR_HPP

#include "interfaces/iroha_internal/block_variant.hpp"
#include "validators/abstract_validator.hpp"
#include "validators/any_block_validator.hpp"
#include "validators/block_validator.hpp"
#include "validators/default_validator.hpp"
#include "validators/field_validator.hpp"
#include "validators/signable_validator.hpp"

namespace shared_model {
  namespace validation {
    class BlockVariantValidator
        : public shared_model::validation::AbstractValidator<
              shared_model::interface::BlockVariant> {
     public:
      Answer validate(const shared_model::interface::BlockVariant &m) override {
        return validator_.validate(m);
      }

      using Validator = shared_model::validation::BlockValidator<
          shared_model::validation::FieldValidator,
          shared_model::validation::DefaultTransactionValidator,
          shared_model::validation::UnsignedTransactionsCollectionValidator<
              shared_model::validation::DefaultTransactionValidator>>;

      using BlockVarValidator =
          shared_model::validation::SignableModelValidator<
              shared_model::validation::AnyBlockValidator<
                  Validator,
                  shared_model::validation::EmptyBlockValidator<
                      shared_model::validation::FieldValidator>>,
              const shared_model::interface::BlockVariant &,
              shared_model::validation::FieldValidator>;

      BlockVarValidator validator_;
    };

  }  // namespace validation
}  // namespace shared_model

#endif  // IROHA_BLOCK_VARIANT_VALIDATOR_HPP

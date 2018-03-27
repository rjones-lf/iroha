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

#ifndef IROHA_SHARED_MODEL_DEFAULT_VALIDATOR_HPP
#define IROHA_SHARED_MODEL_DEFAULT_VALIDATOR_HPP

#include "validators/block_validator.hpp"
#include "validators/field_validator.hpp"
#include "validators/proposal_validator.hpp"
#include "validators/query_validator.hpp"
#include "validators/signable_validator.hpp"
#include "validators/transaction_validator.hpp"

namespace shared_model {
  namespace validation {
    using DefaultTransactionValidator =
        TransactionValidator<FieldValidator,
                             CommandValidatorVisitor<FieldValidator>>;
    using DefaultQueryValidator =
        QueryValidator<FieldValidator, QueryValidatorVisitor<FieldValidator>>;
    using DefaultProposalValidator =
        ProposalValidator<FieldValidator, DefaultTransactionValidator>;

    using DefaultBlockValidator =
        BlockValidator<FieldValidator, DefaultTransactionValidator>;

    using DefaultSignableTransactionValidator =
        SignableModelValidator<DefaultTransactionValidator,
                               const interface::Transaction &,
                               FieldValidator>;

    using DefaultSignableQueryValidator =
        SignableModelValidator<DefaultQueryValidator,
                               const interface::Query &,
                               FieldValidator>;

    using DefaultSignableProposalValidator =
        SignableModelValidator<DefaultProposalValidator,
                               const interface::Proposal &,
                               FieldValidator>;

    using DefaultSignableBlockValidator =
        SignableModelValidator<DefaultBlockValidator,
                               const interface::Block &,
                               FieldValidator>;
  }  // namespace validation
}  // namespace shared_model

#endif  // IROHA_SHARED_MODEL_DEFAULT_VALIDATOR_HPP

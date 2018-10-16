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
#include "validators/blocks_query_validator.hpp"
#include "validators/field_validator.hpp"
#include "validators/proposal_validator.hpp"
#include "validators/query_validator.hpp"
#include "validators/signable_validator.hpp"
#include "validators/transaction_validator.hpp"
#include "validators/transactions_collection/batch_order_validator.hpp"
#include "validators/transactions_collection/transactions_collection_validator.hpp"

namespace shared_model {
  namespace validation {

    // -----------------------| Transaction validation |------------------------

    /**
     * Transaction validator which checks stateless validation WITHOUT
     * signatures
     */
    using DefaultUnsignedTransactionValidator =
        TransactionValidator<FieldValidator,
                             CommandValidatorVisitor<FieldValidator>>;

    /**
     * Transaction validator which checks stateless validation and signature of
     * transaction
     */
    using DefaultSignedTransactionValidator =
        SignableModelValidator<DefaultUnsignedTransactionValidator,
                               const interface::Transaction &,
                               FieldValidator>;

    /**
     * Same as DefaultSignedTransactionValidator, but checks signatures only if
     * they are present
     */
    using DefaultOptionalSignedTransactionValidator =
        SignableModelValidator<DefaultUnsignedTransactionValidator,
                               const interface::Transaction &,
                               FieldValidator,
                               false>;

    // --------------------------| Query validation |---------------------------

    /**
     * Query validator which checks stateless validation WITHOUT signatures
     */
    using DefaultUnsignedQueryValidator =
        QueryValidator<FieldValidator, QueryValidatorVisitor<FieldValidator>>;

    /**
     * Query validator which checks stateless validation including signatures
     */
    using DefaultSignedQueryValidator =
        SignableModelValidator<DefaultUnsignedQueryValidator,
                               const interface::Query &,
                               FieldValidator>;

    /**
     * Block query validator checks stateless validation WITHOUT signatures
     */
    using DefaultUnsignedBlocksQueryValidator =
        BlocksQueryValidator<FieldValidator>;

    /**
     * Block query validator which checks stateless validation including
     * signatures
     */
    using DefaultSignedBlocksQueryValidator =
        SignableModelValidator<DefaultUnsignedBlocksQueryValidator,
                               const interface::BlocksQuery &,
                               FieldValidator>;

    // ------------| Transactions collection validation |--------------

    /**
     * Transactions collection validator that checks stateless validness of
     * transactions WITHOUT signatures
     */
    using DefaultUnsignedTransactionsValidator =
        TransactionsCollectionValidator<DefaultUnsignedTransactionValidator>;

    /**
     * Transactions collection validator that checks signatures and stateless
     * validness of transactions
     */
    using DefaultSignedTransactionsValidator =
        TransactionsCollectionValidator<DefaultSignedTransactionValidator>;

    /**
     * Proposal validator which checks stateless validation of proposal
     */
    using DefaultProposalValidator =
        ProposalValidator<FieldValidator, DefaultSignedTransactionsValidator>;

    /**
     * Block validator which checks blocks WITHOUT signatures. Note that it does
     * not check transactions' signatures as well
     */
    using DefaultUnsignedBlockValidator =
        BlockValidator<FieldValidator, DefaultUnsignedTransactionsValidator>;

    /**
     * Block validator which checks blocks including signatures
     */
    using DefaultSignedBlockValidator =
        SignableModelValidator<DefaultUnsignedBlockValidator,
                               const interface::Block &,
                               FieldValidator>;
  }  // namespace validation
}  // namespace shared_model

#endif  // IROHA_SHARED_MODEL_DEFAULT_VALIDATOR_HPP

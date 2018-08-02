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

#include "validators/any_block_validator.hpp"
#include "validators/block_validator.hpp"
#include "validators/blocks_query_validator.hpp"
#include "validators/empty_block_validator.hpp"
#include "validators/field_validator.hpp"
#include "validators/proposal_validator.hpp"
#include "validators/query_validator.hpp"
#include "validators/signable_validator.hpp"
#include "validators/transaction_validator.hpp"
#include "validators/transactions_collection/batch_order_validator.hpp"
#include "validators/transactions_collection/signed_transactions_collection_validator.hpp"
#include "validators/transactions_collection/unsigned_transactions_collection_validator.hpp"

namespace shared_model {
  namespace validation {

    // -----------------------| Transaction validation |------------------------

    /**
     * Transaction validator which checks stateless validation WITHOUT
     * signatures
     */
    using DefaultTransactionValidator =
        TransactionValidator<FieldValidator,
                             CommandValidatorVisitor<FieldValidator>>;

    /**
     * Transaction validator which checks stateless validation
     */
    using DefaultSignedTransactionValidator =
        SignableModelValidator<DefaultTransactionValidator,
                               const interface::Transaction &,
                               FieldValidator>;

    // --------------------------| Query validation |---------------------------

    /**
     * Query validator which checks stateless validation WITHOUT signatures
     */
    using DefaultUnsignedQueryValidator =
        QueryValidator<FieldValidator, QueryValidatorVisitor<FieldValidator>>;

    /**
     * Block query validator checks stateless validation WITHOUT signatures
     */
    using DefaultUnsignedBlocksQueryValidator =
        BlocksQueryValidator<FieldValidator>;

    /**
     * Query validator which checks stateless validation including signatures
     */
    using DefaultSignableQueryValidator =
        SignableModelValidator<DefaultUnsignedQueryValidator,
                               const interface::Query &,
                               FieldValidator>;

    /**
     * Block query validator which checks stateless validation including
     * signatures
     */
    using DefaultSignableBlocksQueryValidator =
        SignableModelValidator<DefaultUnsignedBlocksQueryValidator,
                               const interface::BlocksQuery &,
                               FieldValidator>;

    // --------------------------| Block validation |---------------------------

    /**
     * Block validator which checks blocks WITHOUT signatures
     */
    using DefaultUnsignedBlockValidator = BlockValidator<
        FieldValidator,
        DefaultTransactionValidator,
        SignedTransactionsCollectionValidator<DefaultTransactionValidator>>;

    /**
     * Block validator which checks blocks including signatures
     */
    using DefaultSignableBlockValidator =
        SignableModelValidator<DefaultUnsignedBlockValidator,
                               const interface::Block &,
                               FieldValidator>;

    /**
     * @deprecated
     * In https://soramitsu.atlassian.net/browse/IR-1418 should be removed
     */
    using DefaultEmptyBlockValidator = EmptyBlockValidator<FieldValidator>;

    /**
     * @deprecated
     * In https://soramitsu.atlassian.net/browse/IR-1418 should be removed
     */
    using DefaultAnyBlockValidator =
        AnyBlockValidator<DefaultUnsignedBlockValidator,
                          DefaultEmptyBlockValidator>;

    // ------------------------| Proposal validation |--------------------------

    /**
     * Proposal validator which checks stateless validation of proposal
     */
    using DefaultProposalValidator = ProposalValidator<
        FieldValidator,
        DefaultTransactionValidator,
        UnsignedTransactionsCollectionValidator<DefaultTransactionValidator>>;

    // -----------------| Transaction collection validation |-------------------

    /**
     * Check sequence of transactions without signatures
     */
    using DefaultUnsignedTxCollectionValidator =
        UnsignedTransactionsCollectionValidator<DefaultTransactionValidator,
                                                BatchOrderValidator>;

  }  // namespace validation
}  // namespace shared_model

#endif  // IROHA_SHARED_MODEL_DEFAULT_VALIDATOR_HPP

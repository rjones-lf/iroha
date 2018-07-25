/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_SIGNED_TRANSACTIONS_COLLECTION_VALIDATOR_HPP
#define IROHA_SIGNED_TRANSACTIONS_COLLECTION_VALIDATOR_HPP

#include "validators/transactions_collection/any_order_validator.hpp"
#include "validators/transactions_collection/transactions_collection_validator.hpp"

namespace shared_model {
  namespace validation {

    /**
     * Signed transactions collection validator does not allow to any
     * transaction from the collection to be unsigned. Batch logic should be
     * checked
     */
    template <typename TransactionValidator,
              typename OrderValidator = AnyOrderValidator>
    class SignedTransactionsCollectionValidator
        : public TransactionsCollectionValidator<TransactionValidator,
                                                 OrderValidator> {
     public:
      using TransactionsCollectionValidator<
          TransactionValidator,
          OrderValidator>::TransactionsCollectionValidator;
      Answer validatePointers(const interface::types::SharedTxsCollectionType
                                  &transactions) const override;
    };

  }  // namespace validation
}  // namespace shared_model

#endif  // IROHA_SIGNED_TRANSACTIONS_COLLECTION_VALIDATOR_HPP

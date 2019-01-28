/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef IROHA_TRANSACTION_BATCH_VALIDATOR_HPP
#define IROHA_TRANSACTION_BATCH_VALIDATOR_HPP

#include "interfaces/iroha_internal/transaction_batch.hpp"
#include "validators/abstract_validator.hpp"

namespace shared_model {
  namespace validation {

    class BatchValidator
        : public AbstractValidator<interface::TransactionBatch> {
     public:
      Answer validate(const interface::TransactionBatch &batch) const override;

      Answer validate(interface::types::TransactionsForwardCollectionType
                          transactions) const;
    };
  }  // namespace validation
}  // namespace shared_model

#endif  // IROHA_TRANSACTION_BATCH_VALIDATOR_HPP

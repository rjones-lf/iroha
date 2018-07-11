/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "interfaces/iroha_internal/transaction_sequence.hpp"
#include "validators/field_validator.hpp"
#include "validators/transaction_validator.hpp"

namespace shared_model {
  namespace interface {

    types::TransactionsForwardCollectionType
    TransactionSequence::transactions() {
      return transactions_;
    }

    TransactionSequence::TransactionSequence(
        const types::TransactionsForwardCollectionType &transactions)
        : transactions_(transactions) {}

  }  // namespace interface
}  // namespace shared_model

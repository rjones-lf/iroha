/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "validators/transactions_collection/signed_transactions_collection_validator.hpp"

namespace shared_model {
  namespace validation {

    Answer SignedTransactionsCollectionValidator::validate(
        const interface::types::TransactionsForwardCollectionType &transactions)
        const {
      return Answer();
    }

  }  // namespace validation
}  // namespace shared_model

/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_TRANSACTION_SEQUENCE_VALIDATOR_HPP
#define IROHA_TRANSACTION_SEQUENCE_VALIDATOR_HPP

#include "interfaces/common_objects/types.hpp"
#include "validators/answer.hpp"

namespace shared_model {
  namespace validation {

    /**
     * Validator of transaction sequence, this is not fair implementation now,
     * it always returns empty answer
     */
    class TransactionSequenceValidator {
     public:
      /**
       * Validates collection of transactions
       * @param transactions collection of transactions
       * @return Answer containing errors if any
       */
      Answer validate(const interface::types::TransactionForwardCollectionType
                          &transactions) {
        return Answer();
      }
    };

  }  // namespace validation
}  // namespace shared_model

#endif  // IROHA_TRANSACTION_SEQUENCE_VALIDATOR_HPP

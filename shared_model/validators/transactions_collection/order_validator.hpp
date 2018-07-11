/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_ORDER_VALIDATOR_HPP
#define IROHA_ORDER_VALIDATOR_HPP

#include "validators/answer.hpp"
#include "interfaces/common_objects/transaction_sequence_common.hpp"

namespace shared_model {
  namespace validation {
    class OrderValidator {
     public:
      virtual Answer validate(
          const interface::types::TransactionsForwardCollectionType
              &transactions) const = 0;
    };
  }  // namespace validation
}  // namespace shared_model

#endif  // IROHA_ORDER_VALIDATOR_HPP

/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "interfaces/iroha_internal/transaction_sequence.hpp"

namespace shared_model {
  namespace interface {

    bool TransactionComparator::operator()(
        const std::shared_ptr<Transaction> &lhs,
        const std::shared_ptr<Transaction> &rhs) const {
      // implement fair logic here
      return lhs->hash().hex() < rhs->hash().hex();
    }

  }  // namespace interface
}  // namespace shared_model

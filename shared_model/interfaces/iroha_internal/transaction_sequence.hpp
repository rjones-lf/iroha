/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_TRANSACTION_SEQUENCE_HPP
#define IROHA_TRANSACTION_SEQUENCE_HPP

#include <boost/iterator/iterator_facade.hpp>
#include <boost/range/any_range.hpp>
#include <set>
#include "interfaces/transaction.hpp"

namespace shared_model {
  namespace interface {

    struct TransactionComparator {
      bool operator()(const std::shared_ptr<Transaction> &lhs,
                      const std::shared_ptr<Transaction> &rhs) const;
    };

    /**
     * Transaction sequence is the collection of transactions, where
     * transactions from the same batch are placed contiguously
     */
    using TransactionSequence =
        std::set<std::shared_ptr<Transaction>, TransactionComparator>;

  }  // namespace interface
}  // namespace shared_model

#endif  // IROHA_TRANSACTION_SEQUENCE_HPP

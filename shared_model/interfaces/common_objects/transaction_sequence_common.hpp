/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_TRANSACTION_SEQUENCE_COMMON_HPP
#define IROHA_TRANSACTION_SEQUENCE_COMMON_HPP

#include <boost/range/any_range.hpp>

#include "interfaces/transaction.hpp"

namespace shared_model {
  namespace interface {

    namespace types {

      using TransactionsForwardCollectionType =
          boost::any_range<Transaction,
                           boost::forward_traversal_tag,
                           const Transaction &>;
    }
  }  // namespace interface
}  // namespace shared_model

#endif  // IROHA_TRANSACTION_SEQUENCE_COMMON_HPP

/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "interfaces/query_responses/transactions_page_response.hpp"
#include "interfaces/transaction.hpp"

namespace shared_model {
  namespace interface {

    std::string TransactionsPageResponse::toString() const {
      return detail::PrettyStringBuilder()
          .init("TransactionsPageResponse")
          .appendAll(transactions(), [](auto &tx) { return tx.toString(); })
//          .append(nextTxHash().toString())
          .append(std::to_string(allTransactionsSize()))
          .finalize();
    }

    bool TransactionsPageResponse::operator==(const ModelType &rhs) const {
      return transactions() == rhs.transactions();
    }

  }  // namespace interface
}  // namespace shared_model

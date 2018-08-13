/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "interfaces/queries/get_account_transactions.hpp"

namespace shared_model {
  namespace interface {

    std::string GetAccountTransactions::toString() const {
      return detail::PrettyStringBuilder()
          .init("GetAccountTransactions")
          .append("account_id", accountId())
          .finalize();
    }

    bool GetAccountTransactions::operator==(const ModelType &rhs) const {
      return accountId() == rhs.accountId();
    }

  }  // namespace interface
}  // namespace shared_model

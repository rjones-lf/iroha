/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "interfaces/queries/get_account_detail.hpp"

namespace shared_model {
  namespace interface {

    std::string GetAccountDetail::toString() const {
      return detail::PrettyStringBuilder()
          .init("GetAccountDetail")
          .append("account_id", accountId())
          .finalize();
    }

    bool GetAccountDetail::operator==(const ModelType &rhs) const {
      return accountId() == rhs.accountId();
    }

  }  // namespace interface
}  // namespace shared_model

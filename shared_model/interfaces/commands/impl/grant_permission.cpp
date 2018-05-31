/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "interfaces/commands/grant_permission.hpp"

namespace shared_model {
  namespace interface {

    std::string GrantPermission::toString() const {
      return detail::PrettyStringBuilder()
          .init("GrantPermission")
          .append("account_id", accountId())
          .append("permission", permissionName())
          .finalize();
    }

    bool GrantPermission::operator==(const ModelType &rhs) const {
      return accountId() == rhs.accountId()
          and permissionName() == rhs.permissionName();
    }

  }  // namespace interface
}  // namespace shared_model

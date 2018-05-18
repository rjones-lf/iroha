/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "interfaces/commands/create_role.hpp"

namespace shared_model {
  namespace interface {

    std::string CreateRole::toString() const {
      auto roles_set = rolePermissions();
      return detail::PrettyStringBuilder()
          .init("CreateRole")
          .append("role_name", roleName())
          .appendAll(roles_set, [](auto &role) { return role; })
          .finalize();
    }

    bool CreateRole::operator==(const ModelType &rhs) const {
      return roleName() == rhs.roleName()
          and rolePermissions() == rhs.rolePermissions();
    }

  }  // namespace interface
}  // namespace shared_model

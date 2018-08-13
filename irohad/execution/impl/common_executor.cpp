/**
 * Copyright Soramitsu Co., Ltd. 2018 All Rights Reserved.
 * http://soramitsu.co.jp
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "execution/common_executor.hpp"

#include <algorithm>

#include "backend/protobuf/permissions.hpp"
#include "common/types.hpp"

namespace iroha {

  boost::optional<shared_model::interface::RolePermissionSet>
  getAccountPermissions(const std::string &account_id,
                        ametsuchi::WsvQuery &queries) {
    auto roles = queries.getAccountRoles(account_id);
    if (not roles) {
      return boost::none;
    }
    auto r = roles.value();
    shared_model::interface::RolePermissionSet permissions{};
    std::for_each(r.begin(), r.end(), [&permissions, &queries](auto &role) {
      auto perms = queries.getRolePermissions(role);
      if (not perms) {
        return;
      }
      permissions |= *perms;
    });
    return permissions;
  }

  bool checkAccountRolePermission(
      const std::string &account_id,
      ametsuchi::WsvQuery &queries,
      shared_model::interface::permissions::Role permission) {
    auto accountRoles = queries.getAccountRoles(account_id);
    if (not accountRoles)
      return false;
    for (auto accountRole : *accountRoles) {
      auto rolePerms = queries.getRolePermissions(accountRole);
      if (not rolePerms)
        continue;
      if (rolePerms->test(permission)) {
        return true;
      }
    }
    return false;
  }
}  // namespace iroha

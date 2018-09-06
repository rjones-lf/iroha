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
    return queries.getAccountRoles(account_id) | [&queries](const auto &roles) {
      return std::accumulate(
          roles.begin(),
          roles.end(),
          shared_model::interface::RolePermissionSet{},
          [&queries](auto &&permissions, const auto &role) {
            queries.getRolePermissions(role) |
                [&](const auto &perms) { permissions |= perms; };
            return std::forward<shared_model::interface::RolePermissionSet>(
                permissions);
          });
    };
  }

  bool checkAccountRolePermission(
      const std::string &account_id,
      ametsuchi::WsvQuery &queries,
      shared_model::interface::permissions::Role permission) {
    return queries.getAccountRoles(account_id) |
        [&permission, &queries](const auto &accountRoles) {
          return std::any_of(accountRoles.begin(),
                             accountRoles.end(),
                             [&permission, &queries](const auto &role) {
                               return queries.getRolePermissions(role) |
                                   [&permission](const auto &perms) {
                                     return perms.test(permission);
                                   };
                             });
        };
  }
}  // namespace iroha

/**
 * Copyright Soramitsu Co., Ltd. 2017 All Rights Reserved.
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

#ifndef IROHA_SHARED_MODEL_CREATE_ROLE_HPP
#define IROHA_SHARED_MODEL_CREATE_ROLE_HPP

#include <numeric>
#include <set>

#include "interfaces/base/model_primitive.hpp"
#include "interfaces/common_objects/types.hpp"

namespace shared_model {
  namespace interface {
    /**
     * Create new role in Iroha
     */
    class CreateRole : public ModelPrimitive<CreateRole> {
     public:
      /**
       * @return Id of the domain to create
       */
      virtual const types::RoleIdType &roleName() const = 0;
      /// Set of Permissions to insure the order for consistent hash
      using PermissionsType = std::set<types::PermissionNameType>;
      /**
       * @return permissions associated with the role
       */
      virtual const PermissionsType &rolePermissions() const = 0;

      std::string toString() const override {
        auto roles_set = rolePermissions();
        return detail::PrettyStringBuilder()
            .init("CreateRole")
            .append("role_name", roleName())
            .appendAll(roles_set, [](auto &role) { return role; })
            .finalize();
      }

      bool operator==(const ModelType &rhs) const override {
        return roleName() == rhs.roleName()
            and rolePermissions() == rhs.rolePermissions();
      }
    };
  }  // namespace interface
}  // namespace shared_model

#endif  // IROHA_SHARED_MODEL_CREATE_ROLE_HPP

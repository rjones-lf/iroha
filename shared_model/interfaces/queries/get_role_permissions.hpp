/**
 * Copyright Soramitsu Co., Ltd. 2017 All Rights Reserved.
 * http://soramitsu.co.jp
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copyof the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef IROHA_SHARED_MODEL_GET_ROLE_PERMISSIONS_HPP
#define IROHA_SHARED_MODEL_GET_ROLE_PERMISSIONS_HPP

#include "interfaces/base/primitive.hpp"
#include "interfaces/common_objects/types.hpp"

#ifndef DISABLE_BACKWARD
#include "model/queries/get_roles.hpp"
#endif

namespace shared_model {
  namespace interface {

    /**
     * Get all permissions related to specific role
     */
    class GetRolePermissions : public PRIMITIVE(GetRolePermissions) {
     public:
      /**
       * @return role identifier containing requested permissions
       */
      virtual const types::RoleIdType &roleId() const = 0;

#ifndef DISABLE_BACKWARD
      OldModelType *makeOldModel() const override {
        auto oldModel = new iroha::model::GetRolePermissions;
        oldModel->role_id = roleId();
        return oldModel;
      }

#endif

      std::string toString() const override {
        return detail::PrettyStringBuilder()
            .init("GetRolePermissions")
            .append("role_id", roleId())
            .finalize();
      }

      bool operator==(const ModelType &rhs) const override {
        return roleId() == rhs.roleId();
      }
    };
  }  // namespace interface
}  // namespace shared_model

#endif  // IROHA_SHARED_MODEL_GET_ROLES_HPP

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

#ifndef IROHA_SHARED_MODEL_ROLE_PERMISSIONS_RESPONSE_HPP
#define IROHA_SHARED_MODEL_ROLE_PERMISSIONS_RESPONSE_HPP

#include "interfaces/base/model_primitive.hpp"
#include "interfaces/common_objects/types.hpp"
#include "interfaces/permissions.hpp"

namespace shared_model {
  namespace interface {
    /**
     * Response with all permissions related to role
     */
    class RolePermissionsResponse
        : public ModelPrimitive<RolePermissionsResponse> {
     public:
      /**
       * @return role permissions
       */
      virtual const RolePermissionSet &rolePermissions() const = 0;

      /**
       * Stringify the data.
       * @return string representation of data.
       */
      std::string toString() const override = 0;

      bool operator==(const ModelType &rhs) const override;
    };
  }  // namespace interface
}  // namespace shared_model
#endif  // IROHA_SHARED_MODEL_ROLE_PERMISSIONS_RESPONSE_HPP

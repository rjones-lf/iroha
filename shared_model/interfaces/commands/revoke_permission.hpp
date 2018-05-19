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

#ifndef IROHA_SHARED_MODEL_REVOKE_PERMISSION_HPP
#define IROHA_SHARED_MODEL_REVOKE_PERMISSION_HPP

#include "interfaces/base/model_primitive.hpp"
#include "interfaces/common_objects/types.hpp"

namespace shared_model {
  namespace interface {
    /**
     * Revoke permission from account
     */
    class RevokePermission : public ModelPrimitive<RevokePermission> {
     public:
      /**
       * @return account from which revoke permission
       */
      virtual const types::AccountIdType &accountId() const = 0;

      /**
       * @return Permission to revoke
       */
      virtual const types::PermissionNameType &permissionName() const = 0;

      std::string toString() const override {
        return detail::PrettyStringBuilder()
            .init("RevokePermission")
            .append("account_id", accountId())
            .append("permission", permissionName())
            .finalize();
      }

      bool operator==(const ModelType &rhs) const override {
        return accountId() == rhs.accountId()
            and permissionName() == rhs.permissionName();
      }
    };
  }  // namespace interface
}  // namespace shared_model

#endif  // IROHA_SHARED_MODEL_REVOKE_PERMISSION_HPP

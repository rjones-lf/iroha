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

#ifndef IROHA_PROTO_GET_ROLE_PERMISSIONS_H
#define IROHA_PROTO_GET_ROLE_PERMISSIONS_H

#include "interfaces/queries/get_role_permissions.hpp"

#include "queries.pb.h"
#include "utils/lazy_initializer.hpp"
#include "utils/reference_holder.hpp"

namespace shared_model {
  namespace proto {
    class GetRolePermissions final
        : public CopyableProto<interface::GetRolePermissions,
                               iroha::protocol::Query,
                               GetRolePermissions> {
     public:
      template <typename QueryType>
      explicit GetRolePermissions(QueryType &&query)
          : CopyableProto(std::forward<QueryType>(query)),
            role_permissions_(detail::makeReferenceGenerator(
                &proto_->payload(),
                &iroha::protocol::Query::Payload::get_role_permissions)) {}

      GetRolePermissions(const GetRolePermissions &o)
          : GetRolePermissions(o.proto_) {}

      GetRolePermissions(GetRolePermissions &&o) noexcept
          : GetRolePermissions(std::move(o.proto_)) {}

      const interface::types::RoleIdType &roleId() const override {
        return role_permissions_->role_id();
      }

     private:
      // ------------------------------| fields |-------------------------------
      const detail::LazyInitializer<const iroha::protocol::GetRolePermissions &>
          role_permissions_;
    };

  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_PROTO_GET_ROLE_PERMISSIONS_H

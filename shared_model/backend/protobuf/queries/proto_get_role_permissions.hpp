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

#include "backend/protobuf/common_objects/trivial_proto.hpp"
#include "interfaces/queries/get_role_permissions.hpp"
#include "queries.pb.h"

namespace shared_model {
  namespace proto {
    class GetRolePermissions final
        : public CopyableProto<interface::GetRolePermissions,
                               iroha::protocol::Query,
                               GetRolePermissions> {
     public:
      template <typename QueryType>
      explicit GetRolePermissions(QueryType &&query);

      GetRolePermissions(const GetRolePermissions &o);

      GetRolePermissions(GetRolePermissions &&o) noexcept;

      const interface::types::RoleIdType &roleId() const override;

     private:
      // ------------------------------| fields |-------------------------------
      const iroha::protocol::GetRolePermissions &role_permissions_;
    };

  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_PROTO_GET_ROLE_PERMISSIONS_H

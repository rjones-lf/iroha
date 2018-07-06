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

#ifndef IROHA_PROTO_CREATE_ROLE_HPP
#define IROHA_PROTO_CREATE_ROLE_HPP

#include "backend/protobuf/common_objects/trivial_proto.hpp"
#include "commands.pb.h"
#include "interfaces/commands/create_role.hpp"
#include "interfaces/permissions.hpp"

namespace shared_model {
  namespace proto {
    class CreateRole final : public CopyableProto<interface::CreateRole,
                                                  iroha::protocol::Command,
                                                  CreateRole> {
     public:
      template <typename CommandType>
      explicit CreateRole(CommandType &&command);

      CreateRole(const CreateRole &o);

      CreateRole(CreateRole &&o) noexcept;

      const interface::types::RoleIdType &roleName() const override;

      const interface::RolePermissionSet &rolePermissions() const override;

      std::string toString() const override;

     private:
      // lazy
      template <typename Value>
      using Lazy = detail::LazyInitializer<Value>;

      const iroha::protocol::CreateRole &create_role_;

      const Lazy<interface::RolePermissionSet> role_permissions_;
    };
  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_PROTO_CREATE_ROLE_HPP

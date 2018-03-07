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

#include <boost/range/numeric.hpp>
#include "interfaces/commands/create_role.hpp"

namespace shared_model {
  namespace proto {
    class CreateRole final : public CopyableProto<interface::CreateRole,
                                                  iroha::protocol::Command,
                                                  CreateRole> {
     public:
      template <typename CommandType>
      explicit CreateRole(CommandType &&command)
          : CopyableProto(std::forward<CommandType>(command)) {}

      CreateRole(const CreateRole &o) : CreateRole(o.proto_) {}

      CreateRole(CreateRole &&o) noexcept : CreateRole(std::move(o.proto_)) {}

      const interface::types::RoleIdType &roleName() const override {
        return create_role_.role_name();
      }

      const PermissionsType &rolePermissions() const override {
        return *role_permissions_;
      }

     private:
      // lazy
      template <typename Value>
      using Lazy = detail::LazyInitializer<Value>;

      const iroha::protocol::CreateRole &create_role_{proto_->create_role()};

      const Lazy<PermissionsType> role_permissions_{[this] {
        return boost::accumulate(
            create_role_.permissions(),
            PermissionsType{},
            [](auto &&acc, const auto &perm) {
              acc.insert(iroha::protocol::RolePermission_Name(
                  static_cast<iroha::protocol::RolePermission>(perm)));
              return std::forward<decltype(acc)>(acc);
            });
      }};
    };
  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_PROTO_CREATE_ROLE_HPP

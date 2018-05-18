/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "backend/protobuf/commands/proto_create_role.hpp"

#include <boost/range/numeric.hpp>

namespace shared_model {
  namespace proto {

    template <typename CommandType>
    CreateRole::CreateRole(CommandType &&command)
        : CopyableProto(std::forward<CommandType>(command)),
          create_role_{proto_->create_role()},
          role_permissions_{[this] {
            return boost::accumulate(
                create_role_.permissions(),
                PermissionsType{},
                [](auto &&acc, const auto &perm) {
                  acc.insert(iroha::protocol::RolePermission_Name(
                      static_cast<iroha::protocol::RolePermission>(perm)));
                  return std::forward<decltype(acc)>(acc);
                });
          }} {}

    template CreateRole::CreateRole(iroha::protocol::Command &command);
    template CreateRole::CreateRole(const iroha::protocol::Command &command);

    CreateRole::CreateRole(const CreateRole &o) : CreateRole(o.proto_) {}

    CreateRole::CreateRole(CreateRole &&o) noexcept
        : CreateRole(std::move(o.proto_)) {}

    const interface::types::RoleIdType &CreateRole::roleName() const {
      return create_role_.role_name();
    }

    const CreateRole::PermissionsType &CreateRole::rolePermissions() const {
      return *role_permissions_;
    }

  }  // namespace proto
}  // namespace shared_model

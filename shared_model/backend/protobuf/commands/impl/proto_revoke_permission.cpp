/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "backend/protobuf/commands/proto_revoke_permission.hpp"

namespace shared_model {
  namespace proto {

    template <typename CommandType>
    RevokePermission::RevokePermission(CommandType &&command)
        : CopyableProto(std::forward<CommandType>(command)),
          revoke_permission_{proto_->revoke_permission()} {}

    template RevokePermission::RevokePermission(
        RevokePermission::TransportType &);
    template RevokePermission::RevokePermission(
        const RevokePermission::TransportType &);
    template RevokePermission::RevokePermission(
        RevokePermission::TransportType &&);

    RevokePermission::RevokePermission(const RevokePermission &o)
        : RevokePermission(o.proto_) {}

    RevokePermission::RevokePermission(RevokePermission &&o) noexcept
        : RevokePermission(std::move(o.proto_)) {}

    const interface::types::AccountIdType &RevokePermission::accountId() const {
      return revoke_permission_.account_id();
    }

    const interface::types::PermissionNameType &
    RevokePermission::permissionName() const {
      return iroha::protocol::GrantablePermission_Name(
          revoke_permission_.permission());
    }

  }  // namespace proto
}  // namespace shared_model

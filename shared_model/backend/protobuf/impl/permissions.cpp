/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "backend/protobuf/permissions.hpp"

namespace shared_model {
  namespace proto {
    namespace permissions {

      boost::optional<interface::permissions::Role> fromTransport(
          iroha::protocol::RolePermission perm) noexcept {
        return iroha::protocol::RolePermission_IsValid(perm)
            ? boost::optional<interface::permissions::Role>(
                  static_cast<interface::permissions::Role>(perm))
            : boost::none;
      }

      iroha::protocol::RolePermission toTransport(
          interface::permissions::Role r) {
        return static_cast<iroha::protocol::RolePermission>(r);
      }

      std::string toString(interface::permissions::Role r) {
        return iroha::protocol::RolePermission_Name(toTransport(r));
      }

      boost::optional<interface::permissions::Grantable> fromTransport(
          iroha::protocol::GrantablePermission perm) noexcept {
        return iroha::protocol::GrantablePermission_IsValid(perm)
            ? boost::optional<interface::permissions::Grantable>(
                  static_cast<interface::permissions::Grantable>(perm))
            : boost::none;
      }

      iroha::protocol::GrantablePermission toTransport(
          interface::permissions::Grantable r) {
        return static_cast<iroha::protocol::GrantablePermission>(r);
      }

      std::string toString(interface::permissions::Grantable r) {
        return iroha::protocol::GrantablePermission_Name(toTransport(r));
      }
    }  // namespace permissions
  }    // namespace proto
}  // namespace shared_model

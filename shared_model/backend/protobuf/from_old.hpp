/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef SHARED_MODEL_PROTOBUF_FROM_OLD
#define SHARED_MODEL_PROTOBUF_FROM_OLD

#include <set>
#include <string>
#include "backend/protobuf/permissions.hpp"

namespace shared_model {
  namespace interface {
    namespace permissions {
      static inline Role fromOldR(const std::string &s) {
        iroha::protocol::RolePermission role;
        if (not RolePermission_Parse(s, &role)) {
          throw std::invalid_argument(s);
        }
        return proto::permissions::fromTransport(role);
      }

      static inline RolePermissionSet fromOldR(const std::set<std::string> &s) {
        RolePermissionSet set{};
        for (auto &el : s) {
          set.set(fromOldR(el));
        }
        return set;
      }
    }  // namespace permissions
  }    // namespace interface
}  // namespace shared_model

#endif  // SHARED_MODEL_PROTOBUF_FROM_OLD

/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "backend/protobuf/query_responses/proto_role_permissions_response.hpp"
#include <boost/range/numeric.hpp>

namespace shared_model {
  namespace proto {

    template <typename QueryResponseType>
    RolePermissionsResponse::RolePermissionsResponse(
        QueryResponseType &&queryResponse)
        : CopyableProto(std::forward<QueryResponseType>(queryResponse)),
          rolePermissionsResponse_{proto_->role_permissions_response()},
          rolePermissions_{[this] {
            return boost::accumulate(
                rolePermissionsResponse_.permissions(),
                PermissionNameCollectionType{},
                [](auto &&permissions, const auto &permission) {
                  permissions.emplace_back(permission);
                  return std::move(permissions);
                });
          }} {}

    template RolePermissionsResponse::RolePermissionsResponse(
        RolePermissionsResponse::TransportType &);
    template RolePermissionsResponse::RolePermissionsResponse(
        const RolePermissionsResponse::TransportType &);
    template RolePermissionsResponse::RolePermissionsResponse(
        RolePermissionsResponse::TransportType &&);

    RolePermissionsResponse::RolePermissionsResponse(
        const RolePermissionsResponse &o)
        : RolePermissionsResponse(o.proto_) {}

    RolePermissionsResponse::RolePermissionsResponse(
        RolePermissionsResponse &&o)
        : RolePermissionsResponse(std::move(o.proto_)) {}

    const RolePermissionsResponse::PermissionNameCollectionType &
    RolePermissionsResponse::rolePermissions() const {
      return *rolePermissions_;
    }

  }  // namespace proto
}  // namespace shared_model

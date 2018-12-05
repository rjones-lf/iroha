/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_SHARED_MODEL_PROTO_ROLE_PERMISSIONS_RESPONSE_HPP
#define IROHA_SHARED_MODEL_PROTO_ROLE_PERMISSIONS_RESPONSE_HPP

#include "backend/protobuf/common_objects/trivial_proto.hpp"
#include "interfaces/query_responses/role_permissions.hpp"
#include "qry_responses.pb.h"
#include "utils/lazy_initializer.hpp"

namespace shared_model {
  namespace proto {
    class RolePermissionsResponse final
        : public CopyableProto<interface::RolePermissionsResponse,
                               iroha::protocol::QueryResponse,
                               RolePermissionsResponse> {
     public:
      template <typename QueryResponseType>
      explicit RolePermissionsResponse(QueryResponseType &&queryResponse);

      RolePermissionsResponse(const RolePermissionsResponse &o);

      RolePermissionsResponse(RolePermissionsResponse &&o);

      const interface::RolePermissionSet &rolePermissions() const override;

      std::string toString() const override;

     private:
      template <typename T>
      using Lazy = detail::LazyInitializer<T>;

      const iroha::protocol::RolePermissionsResponse &rolePermissionsResponse_;

      const Lazy<interface::RolePermissionSet> rolePermissions_;
    };
  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_SHARED_MODEL_PROTO_ROLE_PERMISSIONS_RESPONSE_HPP

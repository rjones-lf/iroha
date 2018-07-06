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

#ifndef IROHA_SHARED_MODEL_PROTO_ROLE_PERMISSIONS_RESPONSE_HPP
#define IROHA_SHARED_MODEL_PROTO_ROLE_PERMISSIONS_RESPONSE_HPP

#include "backend/protobuf/common_objects/trivial_proto.hpp"
#include "interfaces/query_responses/role_permissions.hpp"
#include "responses.pb.h"
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

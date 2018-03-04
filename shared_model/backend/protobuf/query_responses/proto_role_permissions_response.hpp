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
#include "utils/reference_holder.hpp"

namespace shared_model {
  namespace proto {
    class RolePermissionsResponse final
        : public CopyableProto<interface::RolePermissionsResponse,
                               iroha::protocol::QueryResponse,
                               RolePermissionsResponse> {
     public:
      template <typename QueryResponseType>
      explicit RolePermissionsResponse(QueryResponseType &&queryResponse)
          : CopyableProto(std::forward<QueryResponseType>(queryResponse)) {}

      RolePermissionsResponse(const RolePermissionsResponse &o)
          : RolePermissionsResponse(o.proto_) {}

      RolePermissionsResponse(RolePermissionsResponse &&o)
          : RolePermissionsResponse(std::move(o.proto_)) {}

      const PermissionNameCollectionType &rolePermissions() const override {
        return *rolePermissions_;
      }

     private:
      template <typename T>
      using Lazy = detail::LazyInitializer<T>;

      const iroha::protocol::RolePermissionsResponse &rolePermissionsResponse_{
          proto_->role_permissions_response()};

      const Lazy<PermissionNameCollectionType> rolePermissions_{[this] {
        return boost::accumulate(
            rolePermissionsResponse_.permissions(),
            PermissionNameCollectionType{},
            [](auto &&permissions, const auto &permission) {
              permissions.emplace_back(permission);
              return std::move(permissions);
            });
      }};
    };
  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_SHARED_MODEL_PROTO_ROLE_PERMISSIONS_RESPONSE_HPP

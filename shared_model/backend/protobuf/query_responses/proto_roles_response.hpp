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

#ifndef IROHA_SHARED_MODEL_PROTO_ROLES_RESPONSE_HPP
#define IROHA_SHARED_MODEL_PROTO_ROLES_RESPONSE_HPP

#include "backend/protobuf/common_objects/trivial_proto.hpp"
#include "interfaces/query_responses/roles_response.hpp"
#include "responses.pb.h"
#include "utils/lazy_initializer.hpp"
#include "utils/reference_holder.hpp"

namespace shared_model {
  namespace proto {
    class RolesResponse final
        : public CopyableProto<interface::RolesResponse,
                               iroha::protocol::QueryResponse,
                               RolesResponse> {
     public:
      template <typename QueryResponseType>
      explicit RolesResponse(QueryResponseType &&queryResponse)
          : CopyableProto(std::forward<QueryResponseType>(queryResponse)) {}

      RolesResponse(const RolesResponse &o) : RolesResponse(o.proto_) {}

      RolesResponse(RolesResponse &&o) : RolesResponse(std::move(o.proto_)) {}

      const RolesIdType &roles() const override {
        return *roles_;
      }

     private:
      template <typename T>
      using Lazy = detail::LazyInitializer<T>;

      const iroha::protocol::RolesResponse &rolesResponse_{
          proto_->roles_response()};

      const Lazy<RolesIdType> roles_{[this] {
        return boost::accumulate(rolesResponse_.roles(),
                                 RolesIdType{},
                                 [](auto &&roles, const auto &role) {
                                   roles.emplace_back(role);
                                   return std::move(roles);
                                 });
      }};
    };
  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_SHARED_MODEL_PROTO_ROLES_RESPONSE_HPP

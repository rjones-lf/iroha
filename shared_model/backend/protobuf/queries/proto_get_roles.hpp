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

#ifndef IROHA_PROTO_GET_ROLES_H
#define IROHA_PROTO_GET_ROLES_H

#include "backend/protobuf/common_objects/trivial_proto.hpp"
#include "interfaces/queries/get_roles.hpp"
#include "queries.pb.h"

namespace shared_model {
  namespace proto {
    class GetRoles final : public CopyableProto<interface::GetRoles,
                                                iroha::protocol::Query,
                                                GetRoles> {
     public:
      template <typename QueryType>
      explicit GetRoles(QueryType &&query);

      GetRoles(const GetRoles &o);

      GetRoles(GetRoles &&o) noexcept;
    };

  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_PROTO_GET_ROLES_H

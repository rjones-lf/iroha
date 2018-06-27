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

#ifndef IROHA_ROLES_RESPONSE_HPP
#define IROHA_ROLES_RESPONSE_HPP

#include "model/query_response.hpp"

namespace iroha {
  namespace model {

    /**
     * Response with all permissions related to role
     */
    struct RolePermissionsResponse : QueryResponse {
      /**
       * All role's permissions
       */
      std::vector<int> role_permissions{};
    };

    /**
     * Provide response with all roles of the current system
     */
    struct RolesResponse : public QueryResponse {
      /**
       * Attached roles
       */
      std::vector<std::string> roles{};
    };
  }  // namespace model
}  // namespace iroha

#endif  // IROHA_ROLES_RESPONSE_HPP

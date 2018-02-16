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

#ifndef IROHA_QUERY_RESPONSE_REGISTRATION_HPP
#define IROHA_QUERY_RESPONSE_REGISTRATION_HPP

#include "common/class_handler.hpp"

// ----------| query responses |----------
#include "model/queries/responses/account_assets_response.hpp"
#include "model/queries/responses/account_detail_response.hpp"
#include "model/queries/responses/account_response.hpp"
#include "model/queries/responses/asset_response.hpp"
#include "model/queries/responses/error_response.hpp"
#include "model/queries/responses/roles_response.hpp"
#include "model/queries/responses/signatories_response.hpp"
#include "model/queries/responses/transactions_response.hpp"

/**
 * File contains registration for all query response subclasses
 */

namespace iroha {
  namespace model {

    class QueryResponseRegistry {
     public:
      QueryResponseRegistry() {
        query_response_handler.register_type(typeid(AccountAssetResponse));
        query_response_handler.register_type(typeid(AccountDetailResponse));
        query_response_handler.register_type(typeid(AccountResponse));
        query_response_handler.register_type(typeid(ErrorResponse));
        query_response_handler.register_type(typeid(SignatoriesResponse));
        query_response_handler.register_type(typeid(TransactionsResponse));
        query_response_handler.register_type(typeid(AssetResponse));
        query_response_handler.register_type(typeid(RolesResponse));
        query_response_handler.register_type(typeid(RolePermissionsResponse));
      }

      ClassHandler query_response_handler{};
    };

  }  // namespace model
}  // namespace iroha

#endif  // IROHA_QUERY_RESPONSE_REGISTRATION_HPP

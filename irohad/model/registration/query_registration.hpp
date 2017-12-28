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

#ifndef IROHA_QUERY_REGISTRATION_HPP
#define IROHA_QUERY_REGISTRATION_HPP

#include "common/class_handler.hpp"

// ----------| queries |----------
#include "model/queries/get_account.hpp"
#include "model/queries/get_account_assets.hpp"
#include "model/queries/get_account_detail.hpp"
#include "model/queries/get_asset_info.hpp"
#include "model/queries/get_roles.hpp"
#include "model/queries/get_signatories.hpp"
#include "model/queries/get_account_transactions.hpp"
#include "model/queries/get_account_asset_transactions.hpp"
#include "model/queries/get_transactions.hpp"
#include "model/queries/get_account_transactions.hpp"

/**
 * File contains registration for all query subclasses
 */

namespace iroha {
  namespace model {

    class QueryRegistry {
     public:
      QueryRegistry() {
        query_handler.register_type(typeid(GetAccount));
        query_handler.register_type(typeid(GetAccountAssets));
        query_handler.register_type(typeid(GetAccountDetail));
        query_handler.register_type(typeid(GetSignatories));
        query_handler.register_type(typeid(GetAccountTransactions));
        query_handler.register_type(typeid(GetAccountAssetTransactions));
        query_handler.register_type(typeid(GetTransactions));
        query_handler.register_type(typeid(GetRoles));
        query_handler.register_type(typeid(GetAssetInfo));
        query_handler.register_type(typeid(GetRolePermissions));
      }

      ClassHandler query_handler{};
    };

  }  // namespace model
}  // namespace iroha

#endif  // IROHA_QUERY_REGISTRATION_HPP

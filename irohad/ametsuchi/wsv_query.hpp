/**
 * Copyright Soramitsu Co., Ltd. 2018 All Rights Reserved.
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

#ifndef IROHA_WSV_QUERY_HPP
#define IROHA_WSV_QUERY_HPP

#include <nonstd/optional.hpp>
#include <string>
#include <vector>
#include "common/types.hpp"

#include "interfaces/common_objects/account.hpp"
#include "interfaces/common_objects/account_asset.hpp"
#include "interfaces/common_objects/asset.hpp"
#include "interfaces/common_objects/domain.hpp"
#include "interfaces/iroha_internal/block.hpp"
#include "interfaces/queries/query.hpp"
#include "interfaces/query_responses/query_response.hpp"
#include "interfaces/transaction.hpp"
#include "interfaces/transaction_responses/tx_response.hpp"

namespace iroha {
  namespace ametsuchi {
    /**
     *  Public interface for world state view queries
     */
    class WsvQuery {
     public:
      virtual ~WsvQuery() = default;

      /**
       * Check if permitee has permission on account
       * @param permitee_account_id
       * @param account_id
       * @param permission_id
       * @return true if has permission, false otherwise
       */
      virtual bool hasAccountGrantablePermission(
          const shared_model::interface::types::AccountIdType
              &permitee_account_id,
          const shared_model::interface::types::AccountIdType &account_id,
          const shared_model::interface::types::PermissionNameType
              &permission_id) = 0;

      /**
       * Get iroha domain
       * @param domain_id - id in the system
       * @return Domain if exist, nullopt otherwise
       */
      virtual nonstd::optional<std::shared_ptr<shared_model::interface::Domain>>
      getDomain(
          const shared_model::interface::types::DomainIdType &domain_id) = 0;

      /**
       * Get account's roles
       * @param account_id
       * @return
       */
      virtual nonstd::optional<
          std::vector<shared_model::interface::types::RoleIdType>>
      getAccountRoles(
          const shared_model::interface::types::AccountIdType &account_id) = 0;
      /**
       * Get all permissions of a role
       * @param role_name
       * @return
       */
      virtual nonstd::optional<
          std::vector<shared_model::interface::types::PermissionNameType>>
      getRolePermissions(
          const shared_model::interface::types::RoleIdType &role_name) = 0;

      /**
       * @return All roles currently in the system
       */
      virtual nonstd::optional<
          std::vector<shared_model::interface::types::RoleIdType>>
      getRoles() = 0;

      /**
       * Get account by user account_id
       * @param account_id
       * @return
       */
      virtual nonstd::optional<
          std::shared_ptr<shared_model::interface::Account>>
      getAccount(
          const shared_model::interface::types::AccountIdType &account_id) = 0;

      /**
       * Get accounts information from its key-value storage
       * @param account_id - account to get details about
       * @return optional of account details
       */
      virtual nonstd::optional<std::string> getAccountDetail(
          const std::string &account_id) = 0;

      /**
       * Get signatories of account by user account_id
       * @param account_id
       * @return
       */
      virtual nonstd::optional<
          std::vector<shared_model::interface::types::PubkeyType>>
      getSignatories(
          const shared_model::interface::types::AccountIdType &account_id) = 0;

      /**
       * Get asset by its name
       * @param asset_id
       * @return
       */
      virtual nonstd::optional<std::shared_ptr<shared_model::interface::Asset>>
      getAsset(const shared_model::interface::types::AssetIdType &asset_id) = 0;

      /**
       *
       * @param account_id
       * @param asset_id
       * @return
       */
      virtual nonstd::optional<
          std::shared_ptr<shared_model::interface::AccountAsset>>
      getAccountAsset(
          const shared_model::interface::types::AccountIdType &account_id,
          const shared_model::interface::types::AssetIdType &asset_id) = 0;

      /**
       *
       * @return
       */
      virtual nonstd::optional<
          std::vector<std::shared_ptr<shared_model::interface::Peer>>>
      getPeers() = 0;
    };

  }  // namespace ametsuchi
}  // namespace iroha

#endif  // IROHA_WSV_QUERY_HPP

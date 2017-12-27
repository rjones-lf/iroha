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

#ifndef IROHA_WSV_QUERY_HPP
#define IROHA_WSV_QUERY_HPP

#include <common/types.hpp>
#include <model/account.hpp>
#include <model/account_asset.hpp>
#include <model/asset.hpp>
#include <model/peer.hpp>
#include <nonstd/optional.hpp>
#include <string>
#include <vector>
#include "model/domain.hpp"

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
          const std::string &permitee_account_id,
          const std::string &account_id,
          const std::string &permission_id) = 0;

      /**
       * Get iroha domain
       * @param domain_id - id in the system
       * @return Domain if exist, nullopt otherwise
       */
      virtual nonstd::optional<model::Domain> getDomain(
          const std::string &domain_id) = 0;

      /**
       * Get account's roles
       * @param account_id
       * @return
       */
      virtual nonstd::optional<std::vector<std::string>> getAccountRoles(
          const std::string &account_id) = 0;
      /**
       * Get all permissions of a role
       * @param role_name
       * @return
       */
      virtual nonstd::optional<std::vector<std::string>> getRolePermissions(
          const std::string &role_name) = 0;

      /**
       * @return All roles currently in the system
       */
      virtual nonstd::optional<std::vector<std::string>> getRoles() = 0;
      /**
       * Get account by user account_id
       * @param account_id
       * @return
       */
      virtual nonstd::optional<model::Account> getAccount(
          const std::string &account_id) = 0;

      /**
       * Get accounts information from its key-value storage
       * @param account_id
       * @param creator_account_id
       * @param detail
       * @return
       */
      virtual nonstd::optional<std::string> getAccountDetail(
          const std::string &account_id,
          const std::string &creator_account_id,
          const std::string &detail) = 0;

      /**
       * Get signatories of account by user account_id
       * @param account_id
       * @return
       */
      virtual nonstd::optional<std::vector<pubkey_t>> getSignatories(
          const std::string &account_id) = 0;

      /**
       * Get asset by its name
       * @param asset_id
       * @return
       */
      virtual nonstd::optional<model::Asset> getAsset(
          const std::string &asset_id) = 0;

      /**
       *
       * @param account_id
       * @param asset_id
       * @return
       */
      virtual nonstd::optional<model::AccountAsset> getAccountAsset(
          const std::string &account_id, const std::string &asset_id) = 0;

      /**
       *
       * @return
       */
      virtual nonstd::optional<std::vector<model::Peer>> getPeers() = 0;
    };

  }  // namespace ametsuchi
}  // namespace iroha

#endif  // IROHA_WSV_QUERY_HPP

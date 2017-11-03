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

#ifndef IROHA_WSV_COMMAND_HPP
#define IROHA_WSV_COMMAND_HPP

#include <common/types.hpp>
#include <model/account.hpp>
#include <model/account_asset.hpp>
#include <model/asset.hpp>
#include <model/domain.hpp>
#include <model/peer.hpp>
#include <string>
#include <set>

namespace iroha {
  namespace ametsuchi {

    /**
     * Commands for modifying world state view
     */
    class WsvCommand {
     public:
      virtual ~WsvCommand() = default;

      /**
       * Insert role entity
       * @param role_name
       * @return true if insert successful
       */
      virtual bool insertRole(const std::string &role_name) = 0;

      /**
       * Bind account and role
       * @param account_id
       * @param role_name
       * @return true if insert successful
       */
      virtual bool insertAccountRole(const std::string &account_id,
                                     const std::string &role_name) = 0;


      /**
       * Bind role and permissions
       * @param role_id
       * @param permissions
       * @return true is insert successful, false otherwise
       */
      virtual bool insertRolePermissions(const std::string &role_id,
          const std::set<std::string> &permissions) = 0;

      /**
       * Insert grantable permission
       * @param permittee_account_id to who give the grant permission
       * @param account_id on which account
       * @param permission_id what permission
       * @return true is execution is successful
       */
      virtual bool insertAccountGrantablePermission(
          const std::string &permittee_account_id,
          const std::string &account_id, const std::string &permission_id) = 0;

      /**
       * Delete grantable permission
       * @param permittee_account_id to who the grant permission was previously granted
       * @param account_id on which account
       * @param permission_id what permission
       * @return true is execution is successful
       */
      virtual bool deleteAccountGrantablePermission(
          const std::string &permittee_account_id,
          const std::string &account_id, const std::string &permission_id) = 0;

      /**
       *
       * @param account
       * @return
       */
      virtual bool insertAccount(const model::Account &account) = 0;

      /**
       *
       * @param account
       * @return true if no error occurred, false otherwise
       */
      virtual bool updateAccount(const model::Account &account) = 0;

      /**
       * @param account_id  account in which update key value
       * @param key - key to set
       * @param val - value of the key/value pair
       * @return true if no error occurred, false otherwise
       */
      virtual bool setAccountKV(const std::string &account_id,
                                const std::string &key,
                                const std::string &val) = 0;

      /**
       *
       * @param asset
       * @return
       */
      virtual bool insertAsset(const model::Asset &asset) = 0;

      /**
       * Update or insert account asset
       * @param asset
       * @return
       */
      virtual bool upsertAccountAsset(const model::AccountAsset &asset) = 0;

      /**
       *
       * @param signatory
       * @return
       */
      virtual bool insertSignatory(const pubkey_t &signatory) = 0;

      /**
       * Insert account signatory relationship
       * @param account_id
       * @param signatory
       * @return
       */
      virtual bool insertAccountSignatory(
          const std::string &account_id,
          const pubkey_t &signatory) = 0;

      /**
       * Delete account signatory relationship
       * @param account_id
       * @param signatory
       * @return
       */
      virtual bool deleteAccountSignatory(
          const std::string &account_id,
          const pubkey_t &signatory) = 0;

      /**
       * Delete signatory
       * @param signatory
       * @return
       */
      virtual bool deleteSignatory(
          const pubkey_t &signatory) = 0;

      /**
       *
       * @param peer
       * @return
       */
      virtual bool insertPeer(const model::Peer &peer) = 0;

      /**
       *
       * @param peer
       * @return
       */
      virtual bool deletePeer(const model::Peer &peer) = 0;

      /**
       *
       * @param peer
       * @return
       */
      virtual bool insertDomain(const model::Domain &domain) = 0;
    };

  }  // namespace ametsuchi
}  // namespace iroha

#endif  // IROHA_WSV_COMMAND_HPP

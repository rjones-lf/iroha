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

#ifndef IROHA_WSV_COMMAND_HPP
#define IROHA_WSV_COMMAND_HPP

#include <set>
#include <string>
#include "common/result.hpp"
#include "common/types.hpp"

#include "common/types.hpp"  // for pubkey_t

namespace iroha {

  namespace model {
    struct Asset;
    struct Account;
    struct Domain;
    struct Peer;
    struct AccountAsset;
  }

  namespace ametsuchi {

    /**
     * Error returned by wsv command.
     * It is a string which contains what action has failed (e.g, "failed to
     * insert role"), and an error which was provided by underlying
     * implementation (e.g, database exception info)
     */
    using WsvError = std::string;

    /**
     *  If command is successful, we assume changes are made,
     *  and do not need anything
     *  If something goes wrong, Result will contain WsvError
     *  with additional information
     */
    using WsvCommandResult = expected::Result<void, WsvError>;

    /**
     * Commands for modifying world state view
     */
    class WsvCommand {
     public:
      virtual ~WsvCommand() = default;

      /**
       * Insert role entity
       * @param role_name
       * @return WsvCommandResult, which will contain error in case of failure
       */
      virtual WsvCommandResult insertRole(const std::string &role_name) = 0;

      /**
       * Bind account and role
       * @param account_id
       * @param role_name
       * @return WsvCommandResult, which will contain error in case of failure
       */
      virtual WsvCommandResult insertAccountRole(
          const std::string &account_id, const std::string &role_name) = 0;
      /**
       * Unbind account and role
       * @param account_id
       * @param role_name
       * @return WsvCommandResult, which will contain error in case of failure
       */
      virtual WsvCommandResult deleteAccountRole(
          const std::string &account_id, const std::string &role_name) = 0;

      /**
       * Bind role and permissions
       * @param role_id
       * @param permissions
       * @return WsvCommandResult, which will contain error in case of failure
       */
      virtual WsvCommandResult insertRolePermissions(
          const std::string &role_id,
          const std::set<std::string> &permissions) = 0;

      /**
       * Insert grantable permission
       * @param permittee_account_id to who give the grant permission
       * @param account_id on which account
       * @param permission_id what permission
       * @return WsvCommandResult, which will contain error in case of failure
       */
      virtual WsvCommandResult insertAccountGrantablePermission(
          const std::string &permittee_account_id,
          const std::string &account_id,
          const std::string &permission_id) = 0;

      /**
       * Delete grantable permission
       * @param permittee_account_id to who the grant permission was previously
       * granted
       * @param account_id on which account
       * @param permission_id what permission
       * @return WsvCommandResult, which will contain error in case of failure
       */
      virtual WsvCommandResult deleteAccountGrantablePermission(
          const std::string &permittee_account_id,
          const std::string &account_id,
          const std::string &permission_id) = 0;

      /**
       *git
       * @param account
       * @return WsvCommandResult, which will contain error in case of failure
       */
      virtual WsvCommandResult insertAccount(const model::Account &account) = 0;

      /**
       *
       * @param account
       * @return WsvCommandResult, which will contain error in case of failure
       */
      virtual WsvCommandResult updateAccount(const model::Account &account) = 0;

      /**
       * @param account_id  account in which update key value
       * @param creator_account_id creator's account who wants to update
       * account_id
       * @param key - key to set
       * @param val - value of the key/value pair
       * @return WsvCommandResult, which will contain error in case of failure
       */
      virtual WsvCommandResult setAccountKV(
          const std::string &account_id,
          const std::string &creator_account_id,
          const std::string &key,
          const std::string &val) = 0;

      /**
       *
       * @param asset
       * @return WsvCommandResult, which will contain error in case of failure
       */
      virtual WsvCommandResult insertAsset(const model::Asset &asset) = 0;

      /**
       * Update or insert account asset
       * @param asset
       * @return WsvCommandResult, which will contain error in case of failure
       */
      virtual WsvCommandResult upsertAccountAsset(
          const model::AccountAsset &asset) = 0;

      /**
       *
       * @param signatory
       * @return WsvCommandResult, which will contain error in case of failure
       */
      virtual WsvCommandResult insertSignatory(const pubkey_t &signatory) = 0;

      /**
       * Insert account signatory relationship
       * @param account_id
       * @param signatory
       * @return WsvCommandResult, which will contain error in case of failure
       */
      virtual WsvCommandResult insertAccountSignatory(
          const std::string &account_id, const pubkey_t &signatory) = 0;

      /**
       * Delete account signatory relationship
       * @param account_id
       * @param signatory
       * @return WsvCommandResult, which will contain error in case of failure
       */
      virtual WsvCommandResult deleteAccountSignatory(
          const std::string &account_id, const pubkey_t &signatory) = 0;

      /**
       * Delete signatory
       * @param signatory
       * @return WsvCommandResult, which will contain error in case of failure
       */
      virtual WsvCommandResult deleteSignatory(const pubkey_t &signatory) = 0;

      /**
       *
       * @param peer
       * @return WsvCommandResult, which will contain error in case of failure
       */
      virtual WsvCommandResult insertPeer(const model::Peer &peer) = 0;

      /**
       *
       * @param peer
       * @return WsvCommandResult, which will contain error in case of failure
       */
      virtual WsvCommandResult deletePeer(const model::Peer &peer) = 0;

      /**
       *
       * @param peer
       * @return WsvCommandResult, which will contain error in case of failure
       */
      virtual WsvCommandResult insertDomain(const model::Domain &domain) = 0;
    };

  }  // namespace ametsuchi
}  // namespace iroha

#endif  // IROHA_WSV_COMMAND_HPP

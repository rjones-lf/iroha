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

#include <model/account_asset.hpp>
#include <model/asset.hpp>
#include <model/account.hpp>
#include <model/peer.hpp>
#include <model/domain.hpp>
#include <common/types.hpp>
#include <string>

namespace iroha {
  namespace ametsuchi {

    /**
     * Commands for modifying world state view
     */
    class WsvCommand {
     public:
      virtual ~WsvCommand() = default;

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
      virtual bool insertSignatory(const ed25519::pubkey_t &signatory) = 0;

      /**
      * Insert account signatory relationship
      * @param account_id
      * @param signatory
      * @return
      */
      virtual bool insertAccountSignatory(
          const std::string &account_id,
          const ed25519::pubkey_t &signatory) = 0;

      /**
       * Delete account signatory relationship
       * @param account_id
       * @param signatory
       * @return
       */
      virtual bool deleteAccountSignatory(
          const std::string &account_id,
          const ed25519::pubkey_t &signatory) = 0;

      /**
       * Delete signatory
       * @param signatory
       * @return
       */
      virtual bool deleteSignatory(
          const ed25519::pubkey_t &signatory) = 0;

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

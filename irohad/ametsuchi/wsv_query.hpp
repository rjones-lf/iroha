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

#ifndef IROHA_WSVQUERY_HPP
#define IROHA_WSVQUERY_HPP

#include <common/types.hpp>
#include <dao/dao.hpp>
#include <string>
#include <vector>

namespace iroha {

  namespace ametsuchi {

    /**
     *  Public interface for world state view queries
     */
    class WsvQuery {
     public:
      /**
       * Get peer by it pub key
       * @param pub_key
       * @return Peer DAO
       */
      virtual iroha::dao::Peer get_peer(
          iroha::ed25519::pubkey_t pub_key) = 0;

      /**
       * Get account by it's first public key.
       * @param pub_key
       * @return DAO Account
       */
      virtual dao::Account get_account(
          ed25519::pubkey_t pub_key) = 0;

      /**
       * Get asset by full name. For example USD#soramitsu.co.jp
       * @param full_name of an asset (name#domain)
       * @return DAO Asset
       */
      virtual dao::Asset get_asset(std::string asset_full_name) = 0;

      /**
       * Get domain by domain's full name. For example soramitsu.co.jp
       * @param full_name of a domain
       * @return DAO Domain
       */
      virtual dao::Domain get_domain(std::string domain_full_name) = 0;

      /**
       * Get wallet by wallet_id
       * @param wallet_id
       * @return DAO Wallet
       */
      virtual dao::Wallet get_wallet(std::string wallet_id) = 0;

      /**
       * Get all wallets of a account.
       * @param pub_key of a account
       * @return vector of DAO Wallet
       */
      virtual std::vector<dao::Wallet> get_account_wallets(
          ed25519::pubkey_t pub_key) = 0;

      /**
       * Get all asset of a domain.
       * @param  full_name of a domain
       * @return vector of DAO Asset
       */
      virtual std::vector<dao::Asset> get_domain_assets(
          std::string domain_full_name) = 0;
    };

  }  // namespace ametsuchi

}  // namespace iroha

#endif  // IROHA_WSVQUERY_HPP

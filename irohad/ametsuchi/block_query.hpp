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

#ifndef IROHA_BLOCK_QUERY_HPP
#define IROHA_BLOCK_QUERY_HPP

#include <model/model.hpp>
#include <rxcpp/rx-observable.hpp>

namespace iroha {

  namespace ametsuchi {
    /**
     * Public interface for queries on blocks and transactions
     */
    class BlockQuery {
     public:
      /**
       * Get all transactions of an account.
       * @param pub_key - account's first public key
       * @return observable of Model Transaction
       */
      virtual rxcpp::observable<model::Transaction> get_account_transactions(
          ed25519::pubkey_t pub_key) = 0;

      /**
       * Get all transactions with a certain asset
       * @param asset_full_name - full name of an asset, i.e. name#domain
       * @return observable of Model Transaction
       */
      virtual rxcpp::observable<model::Transaction> get_asset_transactions(
          std::string asset_full_name) = 0;

      /**
       * Get all transactions of a certain wallet
       * @param wallet_id - unique wallet
       * @return observable of Model Transaction
       */
      virtual rxcpp::observable<model::Transaction> get_wallet_transactions(
          std::string wallet_id) = 0;

      /**
      * Get all blocks with having id in range [from, to].
      * @param from - starting id
      * @param to - ending id
      * @return observable of Model Block
      */
      virtual rxcpp::observable<iroha::model::Block> get_blocks_in_range(
          uint32_t from, uint32_t to) = 0;
    };

  }  // namespace ametsuchi

}  // namespace iroha

#endif  // IROHA_BLOCK_QUERY_HPP

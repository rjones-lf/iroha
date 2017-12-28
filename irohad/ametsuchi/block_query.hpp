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

#include <cmath>

#include <boost/optional.hpp>
#include <rxcpp/rx-observable.hpp>
#include "model/block.hpp"
#include "model/transaction.hpp"
#include "model/queries/pager.hpp"

namespace iroha {
  namespace ametsuchi {
    /**
     * Public interface for queries on blocks and transactions
     */
    class BlockQuery {
     public:
      virtual ~BlockQuery() = default;
      /**
       * Get all transactions of an account.
       * @param account_id - account_id (accountName@domainName)
       * @param pager - pager for transactions
       * @return observable of Model Transaction
       */
      virtual rxcpp::observable<model::Transaction> getAccountTransactions(
          const std::string& account_id,
          const model::Pager& pager = model::Pager{iroha::hash256_t{},
                                                   model::Pager::MAX_PAGER_LIMIT}) = 0;

      /**
       * Get asset transactions of an account.
       * @param account_id - account_id (accountName@domainName)
       * @param assets_id - list of asset_id (assetName#domainName)
       * @param pager - pager for transactions
       * @return observable of Model Transaction
       */
      virtual rxcpp::observable<model::Transaction>
      getAccountAssetTransactions(const std::string& account_id,
                                  const std::vector<std::string>& assets_id,
                                  const model::Pager& pager) = 0;

      /**
       * Get transactions from transactions' hashes
       * @param tx_hashes - transactions' hashes to retrieve
       * @return observable of Model Transaction
       */
      virtual rxcpp::observable<boost::optional<model::Transaction>>
      getTransactions(const std::vector<iroha::hash256_t> &tx_hashes) = 0;

      /**
       * Get given number of blocks starting with given height.
       * @param height - starting height
       * @param count - number of blocks to retrieve
       * @return observable of Model Block
       */
      virtual rxcpp::observable<model::Block> getBlocks(uint32_t height,
                                                        uint32_t count) = 0;

      /**
       * Get all blocks starting from given height.
       * @param from - starting height
       * @return observable of Model Block
       */
      virtual rxcpp::observable<model::Block> getBlocksFrom(
          uint32_t height) = 0;

      /**
       * Get given number of blocks from top.
       * @param count - number of blocks to retrieve
       * @return observable of Model Block
       */
      virtual rxcpp::observable<model::Block> getTopBlocks(uint32_t count) = 0;

      /**
       * Synchronously gets transaction by its hash
       * @param hash - hash to search
       * @return transaction or boost::none
       */
      virtual boost::optional<model::Transaction> getTxByHashSync(
          const std::string &hash) = 0;
    };
  }  // namespace ametsuchi
}  // namespace iroha

#endif  // IROHA_BLOCK_QUERY_HPP

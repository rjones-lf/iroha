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

#include <boost/optional.hpp>
#include <cmath>
#include <rxcpp/rx-observable.hpp>

#include "common/types.hpp"
#include "interfaces/iroha_internal/block.hpp"
#include "interfaces/transaction.hpp"

namespace iroha {

  namespace ametsuchi {
    /**
     * Public interface for queries on blocks and transactions
     */
    class BlockQuery {
    protected:
      using wTransaction =
          std::shared_ptr<shared_model::interface::Transaction>;
      using wBlock = std::shared_ptr<shared_model::interface::Block>;

     public:
      virtual ~BlockQuery() = default;
      /**
       * Get all transactions of an account.
       * @param account_id - account_id (accountName@domainName)
       * @return observable of Model Transaction
       */
      virtual rxcpp::observable<wTransaction> getAccountTransactions(
          const shared_model::interface::types::AccountIdType &account_id) = 0;

      /**
       * Get asset transactions of an account.
       * @param account_id - account_id (accountName@domainName)
       * @param asset_id - asset_id (assetName#domainName)
       * @return observable of Model Transaction
       */
      virtual rxcpp::observable<wTransaction> getAccountAssetTransactions(
          const shared_model::interface::types::AccountIdType &account_id,
          const shared_model::interface::types::AssetIdType &asset_id) = 0;

      /**
       * Get transactions from transactions' hashes
       * @param tx_hashes - transactions' hashes to retrieve
       * @return observable of Model Transaction
       */
      virtual rxcpp::observable<boost::optional<wTransaction>> getTransactions(
          const std::vector<shared_model::crypto::Hash> &tx_hashes) = 0;

      /**
       * Get given number of blocks starting with given height.
       * @param height - starting height
       * @param count - number of blocks to retrieve
       * @return observable of Model Block
       */
      virtual rxcpp::observable<wBlock> getBlocks(shared_model::interface::types::HeightType height,
                                                  uint32_t count) = 0;

      /**
       * Get all blocks starting from given height.
       * @param from - starting height
       * @return observable of Model Block
       */
      virtual rxcpp::observable<wBlock> getBlocksFrom(shared_model::interface::types::HeightType height) = 0;

      /**
       * Get given number of blocks from top.
       * @param count - number of blocks to retrieve
       * @return observable of Model Block
       */
      virtual rxcpp::observable<wBlock> getTopBlocks(uint32_t count) = 0;

      /**
       * Get height of the top block.
       * @return height
       */
      virtual uint32_t getTopBlockHeight() = 0;

      /**
       * Synchronously gets transaction by its hash
       * @param hash - hash to search
       * @return transaction or boost::none
       */
      virtual boost::optional<wTransaction> getTxByHashSync(
          const shared_model::crypto::Hash &hash) = 0;

      /**
       * Synchronously checks whether transaction
       * with given hash is present in any block
       * @param hash - transaction hash
       * @return true if transaction exists, false otherwise
       */
      virtual bool hasTxWithHash(const shared_model::crypto::Hash &hash) = 0;
    };
  }  // namespace ametsuchi
}  // namespace iroha

#endif  // IROHA_BLOCK_QUERY_HPP

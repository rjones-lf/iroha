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

#ifndef IROHA_REDIS_BLOCK_INDEX_HPP
#define IROHA_REDIS_BLOCK_INDEX_HPP

#include "ametsuchi/impl/block_index.hpp"

#include <boost/format.hpp>
#include <cpp_redis/cpp_redis>

namespace iroha {
  namespace ametsuchi {
    class RedisBlockIndex : public BlockIndex {
     public:
      explicit RedisBlockIndex(cpp_redis::client &client);

      void index(const model::Block &block) override;

     private:
      /**
       * Make index account_id -> list of blocks where his txs exist
       * @param account_id of transaction creator
       * @param height of block
       */
      void indexAccountIdHeight(const std::string &account_id,
                                const std::string &height);

      /**
       * Collect all assets belonging to creator, sender, and receiver
       * to make account_id:height:asset_id -> list of tx indexes (where
       * tx with certain asset is placed in the block)
       * @param account_id of transaction creator
       * @param height of block
       * @param index of transaction in the block
       * @param commands in the transaction
       */
      void indexAccountAssets(const std::string &account_id,
                             const std::string &height,
                             const std::string &index,
                             const model::Transaction::CommandsType &commands);

      cpp_redis::client &client_;
      /// format strings for index keys
      boost::format account_id_height_, account_id_height_asset_id_;
    };
  }  // namespace ametsuchi
}  // namespace iroha

#endif  // IROHA_REDIS_BLOCK_INDEX_HPP

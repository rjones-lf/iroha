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

#ifndef IROHA_FLAT_FILE_BLOCK_QUERY_HPP
#define IROHA_FLAT_FILE_BLOCK_QUERY_HPP

#include "ametsuchi/block_query.hpp"

#include "ametsuchi/impl/flat_file/flat_file.hpp"
#include "model/converters/json_block_factory.hpp"

namespace iroha {
  namespace ametsuchi {
    class FlatFileBlockQuery : public BlockQuery {
     public:
      explicit FlatFileBlockQuery(FlatFile &block_store);

      rxcpp::observable<model::Transaction> getAccountTransactions(
          std::string account_id) override;

      rxcpp::observable<model::Block> getBlocks(uint32_t height,
                                                uint32_t count) override;

      rxcpp::observable<model::Block> getBlocksFrom(uint32_t height) override;

      rxcpp::observable<model::Block> getTopBlocks(uint32_t count) override;

      rxcpp::observable<model::Transaction> getAccountAssetTransactions(
          std::string account_id, std::string asset_id) override;

     private:
      FlatFile &block_store_;

      model::converters::JsonBlockFactory serializer_;
    };
  }  // namespace ametsuchi
}  // namespace iroha

#endif  // IROHA_FLAT_FILE_BLOCK_QUERY_HPP

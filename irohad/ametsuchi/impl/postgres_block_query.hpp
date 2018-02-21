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

#ifndef IROHA_POSTGRES_FLAT_BLOCK_QUERY_HPP
#define IROHA_POSTGRES_FLAT_BLOCK_QUERY_HPP

#include <pqxx/nontransaction>
#include "ametsuchi/block_query.hpp"
#include "ametsuchi/impl/flat_file/flat_file.hpp"
#include "logger/logger.hpp"
#include "postgres_wsv_common.hpp"

#include "model/converters/json_block_factory.hpp"

#include <boost/optional.hpp>

namespace iroha {
  namespace ametsuchi {

    class FlatFile;

    /**
     * Class which implements BlockQuery with a Redis backend.
     */
    class PostgresBlockQuery : public BlockQuery {
     public:
      PostgresBlockQuery(pqxx::nontransaction &transaction_,
                         FlatFile &file_store);

      rxcpp::observable<std::shared_ptr<shared_model::interface::Transaction>>
      getAccountTransactions(const std::string &account_id) override;

      rxcpp::observable<std::shared_ptr<shared_model::interface::Transaction>>
      getAccountAssetTransactions(const std::string &account_id,
                                  const std::string &asset_id) override;

      rxcpp::observable<boost::optional<
          std::shared_ptr<shared_model::interface::Transaction>>>
      getTransactions(
          const std::vector<shared_model::crypto::Hash> &tx_hashes) override;

      boost::optional<std::shared_ptr<shared_model::interface::Transaction>>
      getTxByHashSync(const std::string &hash) override;

      rxcpp::observable<std::shared_ptr<shared_model::interface::Block>>
      getBlocks(uint32_t height, uint32_t count) override;

      rxcpp::observable<std::shared_ptr<shared_model::interface::Block>>
      getBlocksFrom(uint32_t height) override;

      rxcpp::observable<std::shared_ptr<shared_model::interface::Block>>
      getTopBlocks(uint32_t count) override;

     private:
      /**
       * Returns all blocks' ids containing given account id
       * @param account_id
       * @return vector of block ids
       */
      std::vector<shared_model::interface::types::HeightType> getBlockIds(
          const std::string &account_id);

      /**
       * Returns block id which contains transaction with a given hash
       * @param hash - hash of transaction
       * @return block id or boost::none
       */
      boost::optional<shared_model::interface::types::HeightType> getBlockId(
          const std::string &hash);

      /**
       * creates callback to lrange query to redis to supply result to
       * subscriber s
       * @param s
       * @param block_id
       * @return
       */
      std::function<void(pqxx::result &result)> callback(
          const rxcpp::subscriber<
              std::shared_ptr<shared_model::interface::Transaction>> &s,
          uint64_t block_id);

      FlatFile &block_store_;
      pqxx::nontransaction &transaction_;
      logger::Logger log_;
      using ExecuteType = decltype(makeExecuteOptional(transaction_, log_));
      ExecuteType execute_;
      model::converters::JsonBlockFactory serializer_;
    };
  }  // namespace ametsuchi
}  // namespace iroha

#endif  // IROHA_POSTGRES_FLAT_BLOCK_QUERY_HPP

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

#ifndef IROHA_REDIS_FLAT_BLOCK_QUERY_HPP
#define IROHA_REDIS_FLAT_BLOCK_QUERY_HPP

#include <cpp_redis/cpp_redis>
#include "ametsuchi/block_query.hpp"
#include "ametsuchi/impl/flat_file/flat_file.hpp"

#include "model/converters/json_block_factory.hpp"

#include <boost/optional.hpp>

namespace iroha {
  namespace ametsuchi {
    /**
     * Class which implements BlockQuery with a Redis backend.
     */
    class RedisBlockQuery : public BlockQuery {
     public:
      RedisBlockQuery(cpp_redis::client &client, FlatFile &file_store);

      rxcpp::observable<model::Transaction> getAccountTransactions(
          const std::string &account_id, const model::Pager &pager) override;

      rxcpp::observable<model::Transaction> getAccountAssetTransactions(
          const std::string &account_id,
          const std::vector<std::string> &assets_id,
          const model::Pager &pager) override;

      rxcpp::observable<boost::optional<model::Transaction>> getTransactions(
          const std::vector<iroha::hash256_t> &tx_hashes) override;

      boost::optional<model::Transaction> getTxByHashSync(
          const std::string &hash) override;

      rxcpp::observable<model::Block> getBlocks(uint32_t height,
                                                uint32_t count) override;

      rxcpp::observable<model::Block> getBlocksFrom(uint32_t height) override;

      rxcpp::observable<model::Block> getTopBlocks(uint32_t count) override;

     private:
      /**
       * Returns all blocks' ids containing given account id
       * @param account_id
       * @return vector of block ids
       */
      std::vector<iroha::model::Block::BlockHeightType> getBlockIds(
          const std::string &account_id);

      /**
       * @param o - given observable
       * @return observable reversed the elements to emit
       */
      rxcpp::observable<model::Transaction> reverseObservable(
          const rxcpp::observable<model::Transaction> &o) const;

      /**
       * Returns block id which contains transaction with a given hash
       * @param hash - hash of transaction
       * @return block id or boost::none
       */
      boost::optional<iroha::model::Block::BlockHeightType> getBlockId(
          const std::string &hash);

      /**
       * @tparam CommandType - expected command type: TransferAsset or
       * AddAssetQuantity
       * @tparam Predicate - type of predicate (usually lambda, can be induced)
       * @param command - target command such that we want to know it's valid.
       * @param predicate - predicate function to know it's valid in case the
       * type matches.
       * @return true if CommandType matches the command and command satisfies
       * predicate
       */
      template <typename CommandType, typename Predicate>
      bool isCommandValid(const std::shared_ptr<iroha::model::Command> &command,
                          const Predicate &predicate) const {
        if (const auto p = std::dynamic_pointer_cast<CommandType>(command)) {
          return predicate(*p);
        }
        return false;
      }

      /**
       * @param account_id - account identifier
       * @param assets_id - asset identifiers
       * @param command - command to check if it is account asset related
       * command or not
       * @return true if given command is accout asset related command,
       * otherwise not
       */
      bool hasAccountAssetRelatedCommand(
        const std::string &account_id,
        const std::vector<std::string> &assets_id,
        const std::shared_ptr<iroha::model::Command> &command) const;

      /**
       * creates callback to lrange query to redis to supply result to
       * subscriber s
       * @param s
       * @param block_id
       * @return
       */
      std::function<void(cpp_redis::reply &)> callbackToLrange(
          const rxcpp::subscriber<model::Transaction> &s, uint64_t block_id);

      FlatFile &block_store_;
      cpp_redis::client &client_;
      model::converters::JsonBlockFactory serializer_;
    };
  }  // namespace ametsuchi
}  // namespace iroha

#endif  // IROHA_REDIS_FLAT_BLOCK_QUERY_HPP

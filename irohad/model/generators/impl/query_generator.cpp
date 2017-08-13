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
#include "model/generators/query_generator.hpp"

namespace iroha {
  namespace model {
    namespace generators {

      GetAccount QueryGenerator::generateGetAccount(ts64_t timestamp,
                                                    std::string creator,
                                                    uint64_t query_counter,
                                                    std::string account_id) {
        GetAccount query{};
        query.created_ts = timestamp;
        query.creator_account_id = creator;
        query.account_id = account_id;
        query.query_counter = query_counter;
        query.query_hash =
            hash_provider_.get_hash(std::make_shared<GetAccount>(query));
        return query;
      }

      GetAccountAssets QueryGenerator::generateGetAccountAssets(
          ts64_t timestamp, std::string creator, uint64_t query_counter,
          std::string account_id, std::string asset_id) {
        GetAccountAssets query{};
        query.created_ts = timestamp;
        query.creator_account_id = creator;
        query.query_counter = query_counter;
        query.account_id = account_id;
        query.asset_id = asset_id;
        query.query_hash =
            hash_provider_.get_hash(std::make_shared<GetAccountAssets>(query));
        return query;
      }

      GetSignatories QueryGenerator::generateGetSignatories(
          ts64_t timestamp, std::string creator, uint64_t query_counter,
          std::string account_id) {
        GetSignatories query{};
        query.created_ts = timestamp;
        query.creator_account_id = creator;
        query.query_counter = query_counter;
        query.account_id = account_id;
        query.query_hash =
            hash_provider_.get_hash(std::make_shared<GetSignatories>(query));
        return query;
      }

      GetAccountTransactions QueryGenerator::generateGetAccountTransactions(
          ts64_t timestamp, std::string creator, uint64_t query_counter,
          std::string account_id) {
        GetAccountTransactions query{};
        query.created_ts = timestamp;
        query.creator_account_id = creator;
        query.query_counter = query_counter;
        query.account_id = account_id;
        query.query_hash = hash_provider_.get_hash(
            std::make_shared<GetAccountTransactions>(query));
        return query;
      }

    }  // namespace generators
  }    // namespace model
}  // namespace iroha

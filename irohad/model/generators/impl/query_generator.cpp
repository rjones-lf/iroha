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
#include <memory>

#include "model/generators/query_generator.hpp"
#include "crypto/hash.hpp"

namespace iroha {
  namespace model {
    namespace generators {


      void QueryGenerator::setQueryMetaData(std::shared_ptr<Query> query,
                                            ts64_t timestamp,
                                            std::string creator,
                                            uint64_t query_counter) {
        query->creator_account_id = creator;
        query->query_counter = query_counter;
        query->created_ts = timestamp;
      }

      std::shared_ptr<GetAccount> QueryGenerator::generateGetAccount(ts64_t timestamp,
                                                    std::string creator,
                                                    uint64_t query_counter,
                                                    std::string account_id) {
        auto query = std::make_shared<GetAccount>();
        query->created_ts = timestamp;
        query->creator_account_id = creator;
        query->account_id = account_id;
        query->query_counter = query_counter;
        return query;
      }

      std::shared_ptr<GetAccountAssets> QueryGenerator::generateGetAccountAssets(
          ts64_t timestamp, std::string creator, uint64_t query_counter,
          std::string account_id, std::string asset_id) {
        auto query = std::make_shared<GetAccountAssets>();
        query->created_ts = timestamp;
        query->creator_account_id = creator;
        query->query_counter = query_counter;
        query->account_id = account_id;
        query->asset_id = asset_id;
        return query;
      }

      std::shared_ptr<GetSignatories> QueryGenerator::generateGetSignatories(
          ts64_t timestamp, std::string creator, uint64_t query_counter,
          std::string account_id) {
        auto query = std::make_shared<GetSignatories>();
        query->created_ts = timestamp;
        query->creator_account_id = creator;
        query->query_counter = query_counter;
        query->account_id = account_id;
        return query;
      }

      optional_ptr<GetAccountTransactions>
      QueryGenerator::generateGetAccountTransactions(
          ts64_t timestamp, const std::string& creator,
          uint64_t query_counter, const std::string& account_id,
        const model::Pager& pager) const {
        auto query = std::make_shared<GetAccountTransactions>();
        query->created_ts = timestamp;
        query->creator_account_id = creator;
        query->query_counter = query_counter;
        query->account_id = account_id;
        query->pager = pager;
        return query;
      }

      std::shared_ptr<GetAccountAssetTransactions> QueryGenerator::generateGetAccountAssetTransactions(
          ts64_t timestamp, std::string creator, uint64_t query_counter,
          std::string account_id, std::string asset_id) {
        auto query = std::make_shared<GetAccountAssetTransactions>();
        query->created_ts = timestamp;
        query->creator_account_id = creator;
        query->query_counter = query_counter;
        query->account_id = account_id;
        query->asset_id = asset_id;
        return query;
      }

      std::shared_ptr<GetAssetInfo> QueryGenerator::generateGetAssetInfo() {
        auto query = std::make_shared<GetAssetInfo>("coin#test");
        query->created_ts = 0;
        query->creator_account_id = "admin@test";
        query->query_counter = 0;
        return query;
      }

      std::shared_ptr<GetRoles> QueryGenerator::generateGetRoles() {
        auto query = std::make_shared<GetRoles>();
        query->created_ts = 0;
        query->creator_account_id = "admin@test";
        query->query_counter = 0;
        return query;
      }

      std::shared_ptr<GetRolePermissions> QueryGenerator::generateGetRolePermissions() {
        auto query = std::make_shared<GetRolePermissions>("admin");
        query->created_ts = 0;
        query->creator_account_id = "admin@test";
        query->query_counter = 0;
        return query;
      }

    }  // namespace generators
  }    // namespace model
}  // namespace iroha

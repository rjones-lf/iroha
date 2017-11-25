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
#include "model/common.hpp"
#include "model/queries/get_account.hpp"
#include "model/queries/get_account_assets.hpp"
#include "model/queries/get_signatories.hpp"
#include "model/queries/get_transactions.hpp"
#include "model/queries/get_account_transactions.hpp"
#include "model/queries/get_asset_info.hpp"
#include "model/queries/get_roles.hpp"

#ifndef IROHA_QUERY_GENERATOR_HPP
#define IROHA_QUERY_GENERATOR_HPP
namespace iroha {
  namespace model {
    namespace generators {
      class QueryGenerator {
       public:

        std::shared_ptr<GetAccount> generateGetAccount(ts64_t timestamp, std::string creator,
                                      uint64_t query_counter,
                                      std::string account_id);

        std::shared_ptr<GetAccountAssets> generateGetAccountAssets(ts64_t timestamp,
                                                  std::string creator,
                                                  uint64_t query_counter,
                                                  std::string account_id,
                                                  std::string asset_id);
        std::shared_ptr<GetSignatories> generateGetSignatories(ts64_t timestamp,
                                              std::string creator,
                                              uint64_t query_counter,
                                              std::string account_id);

        std::shared_ptr<GetAccountTransactions> generateGetAccountTransactions(
            ts64_t timestamp, std::string creator, uint64_t query_counter,
            std::string account_id);

        std::shared_ptr<GetAccountAssetTransactions> generateGetAccountAssetTransactions(
            ts64_t timestamp, std::string creator, uint64_t query_counter,
            std::string account_id, std::string asset_id);

        optional_ptr<GetAccountTransactions>
        generateGetAccountTransactions(
            ts64_t timestamp,
            const std::string& creator,
            uint64_t query_counter,
            const std::string& account_id,
            const model::Pager& pager) const;

        /**
         * Generate default query GetAssetInfo
         * @return default GetAssetInfo
         */
        std::shared_ptr<GetAssetInfo> generateGetAssetInfo();

        /**
         * Generate default query GetRoles
         * @return default GetRoles
         */
        std::shared_ptr<GetRoles> generateGetRoles();

        /**
         * Generate default query GetRolePermissions
         * @return default GetRolePermissions
         */
        std::shared_ptr<GetRolePermissions> generateGetRolePermissions();

        void setQueryMetaData(std::shared_ptr<Query> query, ts64_t timestamp,
                              std::string creator, uint64_t query_counter);
      };
    }  // namespace generators
  }    // namespace model
}  // namespace iroha

#endif  // IROHA_QUERY_GENERATOR_HPP

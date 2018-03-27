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
#ifndef IROHA_QUERY_EXECUTION_HPP
#define IROHA_QUERY_EXECUTION_HPP

#include "model/query.hpp"
#include "model/query_response.hpp"

#include "model/queries/get_account.hpp"
#include "model/queries/get_account_assets.hpp"
#include "model/queries/get_account_detail.hpp"
#include "model/queries/get_asset_info.hpp"
#include "model/queries/get_roles.hpp"
#include "model/queries/get_signatories.hpp"
#include "model/queries/get_transactions.hpp"

#include "ametsuchi/block_query.hpp"
#include "ametsuchi/wsv_query.hpp"

namespace iroha {
  namespace model {

    /**
     * Converting business objects to protobuf and vice versa
     */
    class QueryProcessingFactory {
     public:
      /**
       * Execute and validate query.
       *
       * @param query
       * @return
       */
      std::shared_ptr<iroha::model::QueryResponse> execute(
          std::shared_ptr<const model::Query> query);
      /**
       *
       * @param wsvQuery
       * @param blockQuery
       */
      QueryProcessingFactory(std::shared_ptr<ametsuchi::WsvQuery> wsvQuery,
                             std::shared_ptr<ametsuchi::BlockQuery> blockQuery);

     private:
      bool validate(const model::GetAssetInfo &query);

      bool validate(const model::GetRoles &query);

      bool validate(const model::GetRolePermissions &query);

      bool validate(const model::GetAccountAssets &query);

      bool validate(const model::GetAccount &query);

      bool validate(const model::GetSignatories &query);

      bool validate(const model::GetAccountTransactions &query);

      bool validate(const model::GetAccountAssetTransactions &query);

      bool validate(const model::GetAccountDetail &query);

      bool validate(const model::GetTransactions &query);

      std::shared_ptr<iroha::model::QueryResponse> executeGetAssetInfo(
          const model::GetAssetInfo &query);

      std::shared_ptr<iroha::model::QueryResponse> executeGetRoles(
          const model::GetRoles &query);

      std::shared_ptr<iroha::model::QueryResponse> executeGetRolePermissions(
          const model::GetRolePermissions &query);

      std::shared_ptr<iroha::model::QueryResponse> executeGetAccountAssets(
          const model::GetAccountAssets &query);

      std::shared_ptr<iroha::model::QueryResponse> executeGetAccountDetail(
          const model::GetAccountDetail &query);

      std::shared_ptr<iroha::model::QueryResponse> executeGetAccount(
          const model::GetAccount &query);

      std::shared_ptr<iroha::model::QueryResponse> executeGetSignatories(
          const model::GetSignatories &query);

      std::shared_ptr<iroha::model::QueryResponse>
      executeGetAccountAssetTransactions(
          const model::GetAccountAssetTransactions &query);

      std::shared_ptr<iroha::model::QueryResponse>
      executeGetAccountTransactions(const model::GetAccountTransactions &query);

      std::shared_ptr<iroha::model::QueryResponse> executeGetTransactions(
          const model::GetTransactions &query);

      std::shared_ptr<ametsuchi::WsvQuery> _wsvQuery;
      std::shared_ptr<ametsuchi::BlockQuery> _blockQuery;
    };

  }  // namespace model
}  // namespace iroha

#endif  // IROHA_QUERY_EXECUTION_HPP

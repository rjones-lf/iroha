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

#ifndef IROHA_PB_QUERY_RESPONSE_FACTORY_HPP
#define IROHA_PB_QUERY_RESPONSE_FACTORY_HPP

#include "model/account_asset.hpp"
#include "model/common.hpp"
#include "model/queries/responses/account_assets_response.hpp"
#include "model/queries/responses/account_detail_response.hpp"
#include "model/queries/responses/account_response.hpp"
#include "model/queries/responses/asset_response.hpp"
#include "model/queries/responses/error_response.hpp"
#include "model/queries/responses/roles_response.hpp"
#include "model/queries/responses/signatories_response.hpp"
#include "model/queries/responses/transactions_response.hpp"
#include "responses.pb.h"

namespace iroha {
  namespace model {
    namespace converters {

      /**
       * Converting business objects to protobuf and vice versa
       */
      class PbQueryResponseFactory {
       public:
        boost::optional<protocol::QueryResponse> serialize(
            const std::shared_ptr<QueryResponse> query_response) const;
        optional_ptr<QueryResponse> deserialize(
            const protocol::QueryResponse &query_response) const;

        protocol::Account serializeAccount(const model::Account &account) const;
        model::Account deserializeAccount(
            const protocol::Account &pb_account) const;

        protocol::AccountResponse serializeAccountResponse(
            const model::AccountResponse &accountResponse) const;
        model::AccountResponse deserializeAccountResponse(
            const protocol::AccountResponse pb_response) const;

        protocol::AccountAsset serializeAccountAsset(
            const model::AccountAsset &account_asset) const;
        model::AccountAsset deserializeAccountAsset(
            const protocol::AccountAsset &account_asset) const;

        protocol::AccountAssetResponse serializeAccountAssetResponse(
            const model::AccountAssetResponse &accountAssetResponse) const;
        model::AccountAssetResponse deserializeAccountAssetResponse(
            const protocol::AccountAssetResponse &account_asset_response) const;

        protocol::AccountDetailResponse serializeAccountDetailResponse(
            const model::AccountDetailResponse &accountDetailResponse) const;
        model::AccountDetailResponse deserializeAccountDetailResponse(
            const protocol::AccountDetailResponse &account_detail_response)
            const;

        protocol::SignatoriesResponse serializeSignatoriesResponse(
            const model::SignatoriesResponse &signatoriesResponse) const;
        model::SignatoriesResponse deserializeSignatoriesResponse(
            const protocol::SignatoriesResponse &signatoriesResponse) const;

        protocol::TransactionsResponse serializeTransactionsResponse(
            const model::TransactionsResponse &transactionsResponse) const;
        model::TransactionsResponse deserializeTransactionsResponse(
            const protocol::TransactionsResponse &tx_response) const;

        protocol::AssetResponse serializeAssetResponse(
            const model::AssetResponse &response) const;
        model::AssetResponse deserializeAssetResponse(
            const protocol::AssetResponse &response) const;

        protocol::RolesResponse serializeRolesResponse(
            const model::RolesResponse &response) const;
        model::RolesResponse deserializeRolesResponse(
            const protocol::RolesResponse &response) const;

        protocol::RolePermissionsResponse serializeRolePermissionsResponse(
            const model::RolePermissionsResponse &response) const;
        model::RolePermissionsResponse deserializeRolePermissionsResponse(
            const protocol::RolePermissionsResponse &response) const;

        protocol::ErrorResponse serializeErrorResponse(
            const model::ErrorResponse &errorResponse) const;
        model::ErrorResponse deserializeErrorResponse(
            const protocol::ErrorResponse &response) const;
      };
    }  // namespace converters
  }    // namespace model
}  // namespace iroha

#endif  // IROHA_PB_QUERY_RESPONSE_FACTORY_HPP

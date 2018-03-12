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

#ifndef IROHA_PROTO_QUERY_RESPONSE_BUILDER_TEMPLATE_HPP
#define IROHA_PROTO_QUERY_RESPONSE_BUILDER_TEMPLATE_HPP

#include "backend/protobuf/query_responses/proto_query_response.hpp"
#include "builders/protobuf/helpers.hpp"
#include "interfaces/common_objects/types.hpp"
#include "responses.pb.h"

namespace shared_model {
  namespace proto {

    /**
     * Template query response builder for creating new types of query response
     * builders by means of replacing template parameters
     * @tparam S -- field counter for checking that all required fields are
     * set
     * @tparam BT -- build type of built object returned by build method
     */
    template <int S = 0, typename BT = QueryResponse>
    class TemplateQueryResponseBuilder {
     private:
      template <int, typename>
      friend class TemplateQueryResponseBuilder;

      enum RequiredFields { QueryResponseField, QueryHash, TOTAL };

      template <int s>
      using NextBuilder = TemplateQueryResponseBuilder<S | (1 << s), BT>;

      using ProtoQueryResponse = iroha::protocol::QueryResponse;

      template <int Sp>
      TemplateQueryResponseBuilder(
          const TemplateQueryResponseBuilder<Sp, BT> &o)
          : query_response_(o.query_response_) {}

      /**
       * Make transformation on copied content
       * @tparam Transformation - callable type for changing the copy
       * @param t - transform function for proto object
       * @return new builder with updated state
       */
      template <int Fields, typename Transformation>
      auto transform(Transformation t) const {
        NextBuilder<Fields> copy = *this;
        t(copy.query_response_);
        return copy;
      }

      /**
       * Make query field transformation on copied object
       * @tparam Transformation - callable type for changing query
       * @param t - transform function for proto query
       * @return new builder with set query
       */
      template <typename Transformation>
      auto queryResponseField(Transformation t) const {
        NextBuilder<QueryResponseField> copy = *this;
        t(copy.query_response_);
        return copy;
      }

     public:
      TemplateQueryResponseBuilder() = default;

      auto accountAssetResponse(
          const interface::types::AssetIdType &asset_id,
          const interface::types::AccountIdType &account_id,
          const std::string &amount) const {
        return queryResponseField([&](auto proto_query_response) {
          iroha::protocol::AccountAssetResponse *query_response =
              proto_query_response.mutable_account_assets_response();

          query_response->mutable_account_asset()->set_account_id(account_id);
          query_response->mutable_account_asset()->set_asset_id(asset_id);
          initializeProtobufAmount(
              query_response->mutable_account_asset()->mutable_balance(),
              amount);
        });
      }

      //      auto accountDetailResponse(
      //          const interface::AccountDetailResponse &account_detail) const
      //          {
      //        return queryResponseField([&](auto proto_query_response) {
      //          auto query_response =
      //              proto_query_response->mutable_account_detail_response();
      //          query_response->set_account_detail_response(account_detail);
      //        });
      //      }
      //
      //      auto errorQueryResponse(
      //          const interface::ErrorQueryResponse &error) const {
      //        return queryResponseField([&](auto proto_query_response) {
      //          auto query_response =
      //          proto_query_response->mutable_error_response();
      //          query_response->set_error_response(error);
      //        });
      //      }
      //
      //      auto signatoriesResponse(
      //          const interface::SignatoriesResponse &signatories) const {
      //        return queryResponseField([&](auto proto_query_response) {
      //          auto query_response =
      //              proto_query_response->mutable_signatories_response();
      //          query_response->set_signatories_response(signatories);
      //        });
      //      }
      //
      //      auto transactionsResponse(
      //          const interface::TransactionsResponse &transactions) const {
      //        return queryResponseField([&](auto proto_query_response) {
      //          auto query_response =
      //              proto_query_response->mutable_transactions_response();
      //          query_response->set_transactions_response(transactions);
      //        });
      //      }
      //
      //      auto assetResponse(const interface::AssetResponse &asset) const {
      //        return queryResponseField([&](auto proto_query_response) {
      //          auto query_response =
      //          proto_query_response->mutable_asset_response();
      //          query_response->set_asset_response(asset);
      //        });
      //      }
      //
      //      auto rolesResponse(const interface::RolesResponse &roles) const {
      //        return queryResponseField([&](auto proto_query_response) {
      //          auto query_response =
      //          proto_query_response->mutable_roles_response();
      //          query_response->set_roles_response(roles);
      //        });
      //      }
      //
      //      auto rolePermissionsResponse(
      //          const interface::RolePermissionsResponse &role_permissions)
      //          const {
      //        return queryResponseField([&](auto proto_query_response) {
      //          auto query_response =
      //              proto_query_response->mutable_role_permissions_response();
      //          query_response->set_role_permissions_response(role_permissions);
      //        });
      //      }
      //
      auto queryHash(const interface::types::HashType &query_hash) const {
        return transform<QueryHash>([&](auto proto_query_response) {
          proto_query_response.set_query_hash(query_hash.hex());
        });
      }

      auto build() const {
        static_assert(S == (1 << TOTAL) - 1, "Required fields are not set");
        auto result =
            QueryResponse(iroha::protocol::QueryResponse(query_response_));
        return BT(std::move(result));
      }

      static const int total = RequiredFields::TOTAL;

     private:
      ProtoQueryResponse query_response_;
    };
  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_PROTO_QUERY_RESPONSE_BUILDER_TEMPLATE_HPP

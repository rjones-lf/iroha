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

#ifndef IROHA_PROTO_QUERY_BUILDER_TEMPLATE_HPP
#define IROHA_PROTO_QUERY_BUILDER_TEMPLATE_HPP

#include "backend/protobuf/queries/proto_query.hpp"
#include "builders/protobuf/unsigned_proto.hpp"
#include "interfaces/common_objects/types.hpp"
#include "interfaces/transaction.hpp"
#include "queries.pb.h"
#include "validators/default_validator.hpp"

namespace shared_model {
  namespace proto {

    /**
     * Template query builder for creating new types of query builders by
     * means of replacing template parameters
     * @tparam S -- field counter for checking that all required fields are
     * set
     * @tparam SV -- stateless validator called when build method is invoked
     * @tparam BT -- build type of built object returned by build method
     */
    template <int S = 0,
              typename SV = validation::DefaultQueryValidator,
              typename BT = UnsignedWrapper<Query>>
    class TemplateQueryBuilder {
     private:
      template <int, typename, typename>
      friend class TemplateQueryBuilder;

      enum RequiredFields {
        CreatedTime,
        CreatorAccountId,
        QueryField,
        QueryCounter,
        TOTAL
      };

      template <int s>
      using NextBuilder = TemplateQueryBuilder<S | (1 << s), SV, BT>;

      using ProtoQuery = iroha::protocol::Query;

      template <int Sp>
      TemplateQueryBuilder(const TemplateQueryBuilder<Sp, SV, BT> &o)
          : query_(o.query_), stateless_validator_(o.stateless_validator_) {}

      /**
       * Make transformation on copied content
       * @tparam Transformation - callable type for changing the copy
       * @param t - transform function for proto object
       * @return new builder with updated state
       */
      template <int Fields, typename Transformation>
      auto transform(Transformation t) const {
        NextBuilder<Fields> copy = *this;
        t(copy.query_);
        return copy;
      }

      /**
       * Make query field transformation on copied object
       * @tparam Transformation - callable type for changing query
       * @param t - transform function for proto query
       * @return new builder with set query
       */
      template <typename Transformation>
      auto queryField(Transformation t) const {
        NextBuilder<QueryField> copy = *this;
        t(copy.query_.mutable_payload());
        return copy;
      }

     public:
      TemplateQueryBuilder(const SV &validator = SV())
          : stateless_validator_(validator) {}

      auto createdTime(interface::types::TimestampType created_time) const {
        return transform<CreatedTime>([&](auto &qry) {
          qry.mutable_payload()->set_created_time(created_time);
        });
      }

      auto creatorAccountId(
          const interface::types::AccountIdType &creator_account_id) const {
        return transform<CreatorAccountId>([&](auto &qry) {
          qry.mutable_payload()->set_creator_account_id(creator_account_id);
        });
      }

      auto queryCounter(interface::types::CounterType query_counter) const {
        return transform<QueryCounter>([&](auto &qry) {
          qry.mutable_payload()->set_query_counter(query_counter);
        });
      }

      auto getAccount(const interface::types::AccountIdType &account_id) const {
        return queryField([&](auto proto_query) {
          auto query = proto_query->mutable_get_account();
          query->set_account_id(account_id);
        });
      }

      auto getSignatories(
          const interface::types::AccountIdType &account_id) const {
        return queryField([&](auto proto_query) {
          auto query = proto_query->mutable_get_account_signatories();
          query->set_account_id(account_id);
        });
      }

      auto getAccountTransactions(
          const interface::types::AccountIdType &account_id) const {
        return queryField([&](auto proto_query) {
          auto query = proto_query->mutable_get_account_transactions();
          query->set_account_id(account_id);
        });
      }

      auto getAccountAssetTransactions(
          const interface::types::AccountIdType &account_id,
          const interface::types::AssetIdType &asset_id) const {
        return queryField([&](auto proto_query) {
          auto query = proto_query->mutable_get_account_asset_transactions();
          query->set_account_id(account_id);
          query->set_asset_id(asset_id);
        });
      }

      auto getAccountAssets(
          const interface::types::AccountIdType &account_id,
          const interface::types::AssetIdType &asset_id) const {
        return queryField([&](auto proto_query) {
          auto query = proto_query->mutable_get_account_assets();
          query->set_account_id(account_id);
          query->set_asset_id(asset_id);
        });
      }

      auto getAccountDetail(const interface::types::AccountIdType &account_id) {
        return queryField([&](auto proto_query) {
          auto query = proto_query->mutable_get_account_detail();
          query->set_account_id(account_id);
        });
      }

      auto getRoles() const {
        return queryField(
            [&](auto proto_query) { proto_query->mutable_get_roles(); });
      }

      auto getAssetInfo(const interface::types::AssetIdType &asset_id) const {
        return queryField([&](auto proto_query) {
          auto query = proto_query->mutable_get_asset_info();
          query->set_asset_id(asset_id);
        });
      }

      auto getRolePermissions(
          const interface::types::RoleIdType &role_id) const {
        return queryField([&](auto proto_query) {
          auto query = proto_query->mutable_get_role_permissions();
          query->set_role_id(role_id);
        });
      }

      template <typename Collection>
      auto getTransactions(const Collection &hashes) const {
        return queryField([&](auto proto_query) {
          auto query = proto_query->mutable_get_transactions();
          boost::for_each(hashes, [&query](const auto &hash) {
            query->add_tx_hashes(toBinaryString(hash));
          });
        });
      }

      auto getTransactions(
          std::initializer_list<interface::types::HashType> hashes) const {
        return getTransactions(hashes);
      }

      template <typename... Hash>
      auto getTransactions(const Hash &... hashes) const {
        return getTransactions({hashes...});
      }

      auto build() const {
        static_assert(S == (1 << TOTAL) - 1, "Required fields are not set");
        auto result = Query(iroha::protocol::Query(query_));
        auto answer = stateless_validator_.validate(result);
        if (answer.hasErrors()) {
          throw std::invalid_argument(answer.reason());
        }
        return BT(std::move(result));
      }

      static const int total = RequiredFields::TOTAL;

     private:
      ProtoQuery query_;
      SV stateless_validator_;
    };
  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_PROTO_QUERY_BUILDER_TEMPLATE_HPP

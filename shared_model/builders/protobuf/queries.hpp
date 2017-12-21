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

#ifndef IROHA_PROTO_QUERY_BUILDER_HPP
#define IROHA_PROTO_QUERY_BUILDER_HPP

#include "backend/protobuf/queries/proto_query.hpp"
#include "builders/protobuf/unsigned_proto.hpp"
#include "interfaces/common_objects/types.hpp"
#include "interfaces/transaction.hpp"
#include "queries.pb.h"

namespace shared_model {
  namespace proto {
    template <int S = 0>
    class TemplateQueryBuilder {
     private:
      template <int>
      friend class TemplateQueryBuilder;

      enum RequiredFields {
        CreatedTime,
        CreatorAccountId,
        QueryField,
        QueryCounter,
        TOTAL
      };

      template <int s>
      using NextBuilder = TemplateQueryBuilder<S | (1 << s)>;

      iroha::protocol::Query query_;

      template <int Sp>
      TemplateQueryBuilder(const TemplateQueryBuilder<Sp> &o)
          : query_(o.query_) {}

     public:
      TemplateQueryBuilder() = default;

      NextBuilder<CreatedTime> createdTime(
          interface::types::TimestampType created_time) {
        query_.mutable_payload()->set_created_time(created_time);
        return *this;
      }

      NextBuilder<CreatorAccountId> creatorAccountId(
          const interface::types::AccountIdType &creator_account_id) {
        query_.mutable_payload()->set_creator_account_id(creator_account_id);
        return *this;
      }

      NextBuilder<QueryField> getAccount(
          const interface::types::AccountIdType &account_id) {
        auto query = query_.mutable_payload()->mutable_get_account();
        query->set_account_id(account_id);
        return *this;
      }

      NextBuilder<QueryField> getSignatories(
          const interface::types::AccountIdType &account_id) {
        auto query =
            query_.mutable_payload()->mutable_get_account_signatories();
        query->set_account_id(account_id);
        return *this;
      }

      NextBuilder<QueryField> getAccountTransactions(
          const interface::types::AccountIdType &account_id) {
        auto query =
            query_.mutable_payload()->mutable_get_account_transactions();
        query->set_account_id(account_id);
        return *this;
      }

      NextBuilder<QueryField> getAccountAssetTransactions(
          const interface::types::AccountIdType &account_id,
          const interface::types::AssetIdType &asset_id) {
        auto query =
            query_.mutable_payload()->mutable_get_account_asset_transactions();
        query->set_account_id(account_id);
        query->set_asset_id(asset_id);
        return *this;
      }

      NextBuilder<QueryField> getAccountDetail(
        const interface::types::AccountIdType &account_id,
        const interface::types::DetailType &detail) {
        auto query =
          query_.mutable_payload()->mutable_get_account_detail();
        query->set_account_id(account_id);
        query->set_detail(detail);
        return *this;
      }

      NextBuilder<QueryField> getAccountAssets(
          const interface::types::AccountIdType &account_id,
          const interface::types::AssetIdType &asset_id) {
        auto query = query_.mutable_payload()->mutable_get_account_assets();
        query->set_account_id(account_id);
        query->set_asset_id(asset_id);
        return *this;
      }

      NextBuilder<QueryField> getRoles() {
        query_.mutable_payload()->mutable_get_roles();
        return *this;
      }

      NextBuilder<QueryField> getAssetInfo(
          const interface::types::AssetIdType &asset_id) {
        auto query = query_.mutable_payload()->mutable_get_asset_info();
        query->set_asset_id(asset_id);
        return *this;
      }

      NextBuilder<QueryField> getRolePermissions(
          const interface::types::RoleIdType &role_id) {
        auto query = query_.mutable_payload()->mutable_get_role_permissions();
        query->set_role_id(role_id);
        return *this;
      }

      NextBuilder<QueryField> getTransactions(
          std::initializer_list<interface::Transaction::HashType> hashes) {
        return getTransactions(hashes);
      }

      template <typename... Hash>
      NextBuilder<QueryField> getTransactions(const Hash &... hashes) {
        return getTransactions({hashes...});
      }

      template <typename Collection>
      NextBuilder<QueryField> getTransactions(const Collection &hashes) {
        auto query = query_.mutable_payload()->mutable_get_transactions();
        boost::for_each(hashes, [&query](const auto &hash) {
          query->add_tx_hashes(hash.blob());
        });
        return *this;
      }

      NextBuilder<QueryCounter> queryCounter(
          interface::types::CounterType query_counter) {
        query_.mutable_payload()->set_query_counter(query_counter);
        return *this;
      }

      UnsignedWrapper<Query> build() {
        static_assert(S == (1 << TOTAL) - 1, "Required fields are not set");
        return UnsignedWrapper<Query>(Query(iroha::protocol::Query(query_)));
      }

      static const int total = RequiredFields::TOTAL;
    };

    using QueryBuilder = TemplateQueryBuilder<>;
  }  // namespace proto
}  // namespace shared_model
#endif  // IROHA_PROTO_QUERY_BUILDER_HPP

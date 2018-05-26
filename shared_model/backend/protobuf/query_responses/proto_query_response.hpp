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

#ifndef IROHA_SHARED_MODEL_PROTO_QUERY_RESPONSE_HPP
#define IROHA_SHARED_MODEL_PROTO_QUERY_RESPONSE_HPP

#include "backend/protobuf/query_responses/proto_account_asset_response.hpp"
#include "backend/protobuf/query_responses/proto_account_detail_response.hpp"
#include "backend/protobuf/query_responses/proto_account_response.hpp"
#include "backend/protobuf/query_responses/proto_asset_response.hpp"
#include "backend/protobuf/query_responses/proto_error_query_response.hpp"
#include "backend/protobuf/query_responses/proto_role_permissions_response.hpp"
#include "backend/protobuf/query_responses/proto_roles_response.hpp"
#include "backend/protobuf/query_responses/proto_signatories_response.hpp"
#include "backend/protobuf/query_responses/proto_transaction_response.hpp"

#include "backend/protobuf/common_objects/trivial_proto.hpp"
#include "common/byteutils.hpp"
#include "interfaces/queries/query.hpp"
#include "interfaces/query_responses/query_response.hpp"
#include "responses.pb.h"
#include "utils/lazy_initializer.hpp"
#include "utils/variant_deserializer.hpp"

namespace shared_model {
  namespace proto {
    class QueryResponse final
        : public CopyableProto<interface::QueryResponse,
                               iroha::protocol::QueryResponse,
                               QueryResponse> {
     public:
      /// type of proto variant
      using ProtoQueryResponseVariantType =
          boost::variant<AccountAssetResponse,
                         AccountDetailResponse,
                         AccountResponse,
                         ErrorQueryResponse,
                         SignatoriesResponse,
                         TransactionsResponse,
                         AssetResponse,
                         RolesResponse,
                         RolePermissionsResponse>;

      /// list of types in variant
      using ProtoQueryResponseListType = ProtoQueryResponseVariantType::types;

      template <typename QueryResponseType>
      explicit QueryResponse(QueryResponseType &&queryResponse)
          : CopyableProto(std::forward<QueryResponseType>(queryResponse)) {}

      QueryResponse(const QueryResponse &o) : QueryResponse(o.proto_) {}

      QueryResponse(QueryResponse &&o) noexcept
          : QueryResponse(std::move(o.proto_)) {}

      const QueryResponseVariantType &get() const override {
        return *ivariant_;
      }

      const interface::types::HashType &queryHash() const override {
        return *hash_;
      }

     private:
      template <typename T>
      using Lazy = detail::LazyInitializer<T>;

      using LazyVariantType = Lazy<ProtoQueryResponseVariantType>;

      const LazyVariantType variant_{[this] {
        auto &&ar = *proto_;
        int which =
            ar.GetDescriptor()->FindFieldByNumber(ar.response_case())->index();
        return shared_model::detail::variant_impl<ProtoQueryResponseListType>::
            template load<ProtoQueryResponseVariantType>(
                std::forward<decltype(ar)>(ar), which);
      }};

      const Lazy<QueryResponseVariantType> ivariant_{
          detail::makeLazyInitializer(
              [this] { return QueryResponseVariantType(*variant_); })};

      const Lazy<interface::types::HashType> hash_{[this] {
        return interface::types::HashType(
            iroha::hexstringToBytestring(proto_->query_hash()).get());
      }};
    };
  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_SHARED_MODEL_PROTO_QUERY_RESPONSE_HPP

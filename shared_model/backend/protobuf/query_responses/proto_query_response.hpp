/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
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
#include "backend/protobuf/query_responses/proto_transaction_page_response.hpp"
#include "backend/protobuf/query_responses/proto_transaction_response.hpp"

#include "backend/protobuf/common_objects/trivial_proto.hpp"
#include "interfaces/query_responses/query_response.hpp"
#include "qry_responses.pb.h"
#include "utils/lazy_initializer.hpp"

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
                         RolePermissionsResponse,
                         TransactionsPageResponse>;

      /// list of types in variant
      using ProtoQueryResponseListType = ProtoQueryResponseVariantType::types;

      template <typename QueryResponseType>
      explicit QueryResponse(QueryResponseType &&queryResponse);

      QueryResponse(const QueryResponse &o);

      QueryResponse(QueryResponse &&o) noexcept;

      const QueryResponseVariantType &get() const override;

      const interface::types::HashType &queryHash() const override;

     private:
      template <typename T>
      using Lazy = detail::LazyInitializer<T>;

      using LazyVariantType = Lazy<ProtoQueryResponseVariantType>;

      const LazyVariantType variant_;

      const Lazy<QueryResponseVariantType> ivariant_;

      const Lazy<interface::types::HashType> hash_;
    };
  }  // namespace proto
}  // namespace shared_model

namespace boost {
  extern template class variant<shared_model::proto::AccountAssetResponse,
                                shared_model::proto::AccountDetailResponse,
                                shared_model::proto::AccountResponse,
                                shared_model::proto::ErrorQueryResponse,
                                shared_model::proto::SignatoriesResponse,
                                shared_model::proto::TransactionsResponse,
                                shared_model::proto::AssetResponse,
                                shared_model::proto::RolesResponse,
                                shared_model::proto::RolePermissionsResponse,
                                shared_model::proto::TransactionsPageResponse>;
}

#endif  // IROHA_SHARED_MODEL_PROTO_QUERY_RESPONSE_HPP

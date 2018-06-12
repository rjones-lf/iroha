/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "backend/protobuf/query_responses/proto_query_response.hpp"
#include "common/byteutils.hpp"
#include "utils/variant_deserializer.hpp"

namespace shared_model {
  namespace proto {

    template <typename QueryResponseType>
    QueryResponse::QueryResponse(QueryResponseType &&queryResponse)
        : CopyableProto(std::forward<QueryResponseType>(queryResponse)),
          variant_{[this] {
            auto &&ar = *proto_;
            int which = ar.GetDescriptor()
                            ->FindFieldByNumber(ar.response_case())
                            ->index();
            return shared_model::detail::
                variant_impl<ProtoQueryResponseListType>::template load<
                    ProtoQueryResponseVariantType>(
                    std::forward<decltype(ar)>(ar), which);
          }},
          ivariant_{detail::makeLazyInitializer(
              [this] { return QueryResponseVariantType(*variant_); })},
          hash_{[this] {
            return interface::types::HashType(
                iroha::hexstringToBytestring(proto_->query_hash()).get());
          }} {}

    template QueryResponse::QueryResponse(QueryResponse::TransportType &);
    template QueryResponse::QueryResponse(const QueryResponse::TransportType &);
    template QueryResponse::QueryResponse(QueryResponse::TransportType &&);

    QueryResponse::QueryResponse(const QueryResponse &o)
        : QueryResponse(o.proto_) {}

    QueryResponse::QueryResponse(QueryResponse &&o) noexcept
        : QueryResponse(std::move(o.proto_)) {}

    const QueryResponse::QueryResponseVariantType &QueryResponse::get() const {
      return *ivariant_;
    }

    const interface::types::HashType &QueryResponse::queryHash() const {
      return *hash_;
    }

  }  // namespace proto
}  // namespace shared_model

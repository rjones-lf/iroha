/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_SHARED_MODEL_PROTO_BLOCK_QUERY_RESPONSE_HPP
#define IROHA_SHARED_MODEL_PROTO_BLOCK_QUERY_RESPONSE_HPP

#include "backend/protobuf/query_responses/proto_block_error_response.hpp"
#include "backend/protobuf/query_responses/proto_block_response.hpp"

#include "backend/protobuf/common_objects/trivial_proto.hpp"
#include "interfaces/queries/query.hpp"
#include "interfaces/query_responses/block_query_response.hpp"
#include "responses.pb.h"
#include "utils/lazy_initializer.hpp"
#include "utils/variant_deserializer.hpp"

template <typename... T, typename Archive>
auto loadBlockQueryResponse(Archive &&ar) {
  int which =
      ar.GetDescriptor()->FindFieldByNumber(ar.response_case())->index();
  return shared_model::detail::variant_impl<T...>::template load<
      shared_model::interface::BlockQueryResponse::QueryResponseVariantType>(
      std::forward<Archive>(ar), which);
}

namespace shared_model {
  namespace proto {
    class BlockQueryResponse final
        : public CopyableProto<interface::BlockQueryResponse,
                               iroha::protocol::BlockQueryResponse,
                               BlockQueryResponse> {
     private:
      template <typename... Value>
      using w = boost::variant<const Value &...>;

      template <typename T>
      using Lazy = detail::LazyInitializer<T>;

      using LazyVariantType = Lazy<QueryResponseVariantType>;

     public:
      /// type of proto variant
      using ProtoQueryResponseVariantType =
          w<BlockResponse, BlockErrorResponse>;

      /// list of types in variant
      using ProtoQueryResponseListType = ProtoQueryResponseVariantType::types;

      template <typename QueryResponseType>
      explicit BlockQueryResponse(QueryResponseType &&queryResponse)
          : CopyableProto(std::forward<QueryResponseType>(queryResponse)) {}

      BlockQueryResponse(const BlockQueryResponse &o)
          : BlockQueryResponse(o.proto_) {}

      BlockQueryResponse(BlockQueryResponse &&o) noexcept
          : BlockQueryResponse(std::move(o.proto_)) {}

      const QueryResponseVariantType &get() const override {
        return *variant_;
      }

     private:
      const LazyVariantType variant_{[this] {
        return loadBlockQueryResponse<ProtoQueryResponseListType>(*proto_);
      }};
    };
  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_SHARED_MODEL_PROTO_BLOCK_QUERY_RESPONSE_HPP

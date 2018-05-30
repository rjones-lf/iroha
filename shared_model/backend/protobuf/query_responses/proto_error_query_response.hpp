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

#ifndef IROHA_SHARED_MODEL_PROTO_ERROR_RESPONSE_HPP
#define IROHA_SHARED_MODEL_PROTO_ERROR_RESPONSE_HPP

#include "backend/protobuf/common_objects/trivial_proto.hpp"
#include "backend/protobuf/query_responses/proto_concrete_error_query_response.hpp"
#include "interfaces/query_responses/error_query_response.hpp"
#include "responses.pb.h"
#include "utils/lazy_initializer.hpp"
#include "utils/reference_holder.hpp"
#include "utils/variant_deserializer.hpp"

namespace shared_model {
  namespace proto {
    class ErrorQueryResponse final
        : public CopyableProto<interface::ErrorQueryResponse,
                               iroha::protocol::QueryResponse,
                               ErrorQueryResponse> {
     public:
      /// type of proto variant
      using ProtoQueryErrorResponseVariantType =
          boost::variant<StatelessFailedErrorResponse,
                         StatefulFailedErrorResponse,
                         NoAccountErrorResponse,
                         NoAccountAssetsErrorResponse,
                         NoAccountDetailErrorResponse,
                         NoSignatoriesErrorResponse,
                         NotSupportedErrorResponse,
                         NoAssetErrorResponse,
                         NoRolesErrorResponse>;

      /// list of types in proto variant
      using ProtoQueryErrorResponseListType =
          ProtoQueryErrorResponseVariantType::types;

      template <typename QueryResponseType>
      explicit ErrorQueryResponse(QueryResponseType &&response)
          : CopyableProto(std::forward<QueryResponseType>(response)) {}

      ErrorQueryResponse(const ErrorQueryResponse &o)
          : ErrorQueryResponse(o.proto_) {}

      ErrorQueryResponse(ErrorQueryResponse &&o) noexcept
          : ErrorQueryResponse(std::move(o.proto_)) {}

      const QueryErrorResponseVariantType &get() const override {
        return *ivariant_;
      }

     private:
      /// lazy variant shortcut
      template <typename T>
      using Lazy = detail::LazyInitializer<T>;

      using LazyVariantType = Lazy<ProtoQueryErrorResponseVariantType>;

      const LazyVariantType variant_{[this] {
        auto &&ar = proto_->error_response();

        unsigned which = ar.GetDescriptor()
                             ->FindFieldByName("reason")
                             ->enum_type()
                             ->FindValueByNumber(ar.reason())
                             ->index();
        return shared_model::detail::
            variant_impl<ProtoQueryErrorResponseListType>::template load<
                ProtoQueryErrorResponseVariantType>(
                std::forward<decltype(ar)>(ar), which);
      }};

      const Lazy<QueryErrorResponseVariantType> ivariant_{
          detail::makeLazyInitializer(
              [this] { return QueryErrorResponseVariantType(*variant_); })};
    };
  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_SHARED_MODEL_PROTO_ERROR_RESPONSE_HPP

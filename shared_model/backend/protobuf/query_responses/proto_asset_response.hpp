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

#ifndef IROHA_SHARED_MODEL_PROTO_ASSET_RESPONSE_HPP
#define IROHA_SHARED_MODEL_PROTO_ASSET_RESPONSE_HPP

#include "backend/protobuf/common_objects/asset.hpp"
#include "backend/protobuf/common_objects/trivial_proto.hpp"
#include "interfaces/query_responses/asset_response.hpp"
#include "responses.pb.h"
#include "utils/lazy_initializer.hpp"
#include "utils/reference_holder.hpp"

namespace shared_model {
  namespace proto {
    class AssetResponse final
        : public CopyableProto<interface::AssetResponse,
                               iroha::protocol::QueryResponse,
                               AssetResponse> {
     public:
      template <typename QueryResponseType>
      explicit AssetResponse(QueryResponseType &&queryResponse)
          : CopyableProto(std::forward<QueryResponseType>(queryResponse)),
            assetResponse_(detail::makeReferenceGenerator(
                proto_, &iroha::protocol::QueryResponse::asset_response)),
            asset_([this] { return Asset(assetResponse_->asset()); }) {}

      AssetResponse(const AssetResponse &o) : AssetResponse(o.proto_) {}

      AssetResponse(AssetResponse &&o) : AssetResponse(std::move(o.proto_)) {}

      const Asset &asset() const override { return *asset_; }

     private:
      template <typename T>
      using Lazy = detail::LazyInitializer<T>;

      const Lazy<const iroha::protocol::AssetResponse &> assetResponse_;
      const Lazy<Asset> asset_;
    };
  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_SHARED_MODEL_PROTO_ASSET_RESPONSE_HPP

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

#ifndef IROHA_SHARED_MODEL_PROTO_ACCOUNT_ASSET_RESPONSE_HPP
#define IROHA_SHARED_MODEL_PROTO_ACCOUNT_ASSET_RESPONSE_HPP

#include <boost/range/numeric.hpp>

#include "backend/protobuf/common_objects/account_asset.hpp"
#include "backend/protobuf/common_objects/trivial_proto.hpp"
#include "interfaces/common_objects/types.hpp"
#include "interfaces/query_responses/account_asset_response.hpp"
#include "responses.pb.h"
#include "utils/lazy_initializer.hpp"


namespace shared_model {
  namespace proto {
    class AccountAssetResponse final
        : public CopyableProto<interface::AccountAssetResponse,
                               iroha::protocol::QueryResponse,
                               AccountAssetResponse> {
     public:
      template <typename QueryResponseType>
      explicit AccountAssetResponse(QueryResponseType &&queryResponse)
          : CopyableProto(std::forward<QueryResponseType>(queryResponse)) {}

      AccountAssetResponse(const AccountAssetResponse &o)
          : AccountAssetResponse(o.proto_) {}

      AccountAssetResponse(AccountAssetResponse &&o)
          : AccountAssetResponse(std::move(o.proto_)) {}

      const interface::types::AccountAssetCollectionType accountAssets() const override {
        return *accountAssets_;
      }

     private:
      template <typename T>
      using Lazy = detail::LazyInitializer<T>;

      const iroha::protocol::AccountAssetResponse &accountAssetResponse_{
          proto_->account_assets_response()};

      const Lazy<std::vector<AccountAsset>> accountAssets_{
        [this] {
          return std::vector<proto::AccountAsset>(
              accountAssetResponse_.account_assets().begin(),
              accountAssetResponse_.account_assets().end());
      }};
    };
  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_SHARED_MODEL_PROTO_ACCOUNT_ASSET_RESPONSE_HPP

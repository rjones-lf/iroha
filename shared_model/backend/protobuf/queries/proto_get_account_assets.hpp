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

#ifndef IROHA_PROTO_GET_ACCOUNT_ASSETS_H
#define IROHA_PROTO_GET_ACCOUNT_ASSETS_H

#include "interfaces/queries/get_account_assets.hpp"

#include "queries.pb.h"
#include "utils/lazy_initializer.hpp"
#include "utils/reference_holder.hpp"

namespace shared_model {
  namespace proto {
    class GetAccountAssets final
        : public CopyableProto<interface::GetAccountAssets,
                               iroha::protocol::Query,
                               GetAccountAssets> {
     public:
      template <typename QueryType>
      explicit GetAccountAssets(QueryType &&query)
          : CopyableProto(std::forward<QueryType>(query)),
            account_assets_(detail::makeReferenceGenerator(
                &proto_->payload(),
                &iroha::protocol::Query::Payload::get_account_assets)) {}

      GetAccountAssets(const GetAccountAssets &o)
          : GetAccountAssets(o.proto_) {}

      GetAccountAssets(GetAccountAssets &&o) noexcept
          : GetAccountAssets(std::move(o.proto_)) {}

      const interface::types::AccountIdType &accountId() const override {
        return account_assets_->account_id();
      }

      const interface::types::AssetIdType &assetId() const override {
        return account_assets_->asset_id();
      }

     private:
      // ------------------------------| fields |-------------------------------

      template <typename T>
      using Lazy = detail::LazyInitializer<T>;

      const Lazy<const iroha::protocol::GetAccountAssets &> account_assets_;
    };

  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_PROTO_GET_ACCOUNT_ASSETS_H

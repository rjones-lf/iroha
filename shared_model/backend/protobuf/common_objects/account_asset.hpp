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

#ifndef IROHA_PROTO_ACCOUNT_ASSET_HPP
#define IROHA_PROTO_ACCOUNT_ASSET_HPP

#include "backend/protobuf/common_objects/amount.hpp"
#include "backend/protobuf/common_objects/trivial_proto.hpp"
#include "backend/protobuf/util.hpp"
#include "interfaces/common_objects/account_asset.hpp"
#include "responses.pb.h"
#include "utils/lazy_initializer.hpp"
#include "utils/reference_holder.hpp"

namespace shared_model {
  namespace proto {
    class AccountAsset final
        : public CopyableProto<interface::AccountAsset,
                               iroha::protocol::AccountAsset,
                               AccountAsset> {
     public:
      template <typename AccountAssetType>
      explicit AccountAsset(AccountAssetType &&accountAssetType)
          : CopyableProto(std::forward<AccountAssetType>(accountAssetType)) {}

      AccountAsset(const AccountAsset &o) : AccountAsset(o.proto_) {}

      AccountAsset(AccountAsset &&o) noexcept
          : AccountAsset(std::move(o.proto_)) {}

      const interface::types::AccountIdType &accountId() const override {
        return proto_->account_id();
      }

      const interface::types::AssetIdType &assetId() const override {
        return proto_->asset_id();
      }

      const interface::Amount &balance() const override {
        return *balance_;
      }

     private:
      template <typename T>
      using Lazy = detail::LazyInitializer<T>;

      const Lazy<Amount> balance_{[this] { return Amount(proto_->balance()); }};
    };
  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_PROTO_ACCOUNT_ASSET_HPP

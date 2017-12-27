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

#ifndef IROHA_PROTO_ADD_ASSET_QUANTITY_HPP
#define IROHA_PROTO_ADD_ASSET_QUANTITY_HPP

#include "interfaces/commands/add_asset_quantity.hpp"

#include "backend/protobuf/common_objects/amount.hpp"
#include "backend/protobuf/common_objects/trivial_proto.hpp"
#include "commands.pb.h"
#include "utils/lazy_initializer.hpp"
#include "utils/reference_holder.hpp"

namespace shared_model {
  namespace proto {
    class AddAssetQuantity final
        : public CopyableProto<interface::AddAssetQuantity,
                               iroha::protocol::Command,
                               AddAssetQuantity> {
     public:
      template <typename CommandType>
      explicit AddAssetQuantity(CommandType &&command)
          : CopyableProto(std::forward<CommandType>(command)),
            add_asset_quantity_(detail::makeReferenceGenerator(
                proto_, &iroha::protocol::Command::add_asset_quantity)),
            amount_([this] {
              return proto::Amount(add_asset_quantity_->amount());
            }) {}

      AddAssetQuantity(const AddAssetQuantity &o)
          : AddAssetQuantity(o.proto_) {}

      AddAssetQuantity(AddAssetQuantity &&o) noexcept
          : AddAssetQuantity(std::move(o.proto_)) {}

      const interface::types::AccountIdType &accountId() const override {
        return add_asset_quantity_->account_id();
      }

      const interface::types::AssetIdType &assetId() const override {
        return add_asset_quantity_->asset_id();
      }

      const interface::Amount &amount() const override { return *amount_; }

     private:
      // lazy
      template <typename T>
      using Lazy = detail::LazyInitializer<T>;

      const Lazy<const iroha::protocol::AddAssetQuantity &> add_asset_quantity_;

      const Lazy<proto::Amount> amount_;
    };

  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_PROTO_ADD_ASSET_QUANTITY_HPP

/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "backend/protobuf/commands/proto_add_asset_quantity.hpp"

namespace shared_model {
  namespace proto {

    template <typename CommandType>
    AddAssetQuantity::AddAssetQuantity(CommandType &&command)
        : CopyableProto(std::forward<CommandType>(command)),
          add_asset_quantity_{proto_->add_asset_quantity()},
          amount_{
              [this] { return proto::Amount(add_asset_quantity_.amount()); }} {}

    template AddAssetQuantity::AddAssetQuantity(
        iroha::protocol::Command &command);
    template AddAssetQuantity::AddAssetQuantity(
        const iroha::protocol::Command &command);

    AddAssetQuantity::AddAssetQuantity(const AddAssetQuantity &o)
        : AddAssetQuantity(o.proto_) {}

    AddAssetQuantity::AddAssetQuantity(AddAssetQuantity &&o) noexcept
        : AddAssetQuantity(std::move(o.proto_)) {}

    const interface::types::AccountIdType &AddAssetQuantity::accountId() const {
      return add_asset_quantity_.account_id();
    }

    const interface::types::AssetIdType &AddAssetQuantity::assetId() const {
      return add_asset_quantity_.asset_id();
    }

    const interface::Amount &AddAssetQuantity::amount() const {
      return *amount_;
    }

  }  // namespace proto
}  // namespace shared_model

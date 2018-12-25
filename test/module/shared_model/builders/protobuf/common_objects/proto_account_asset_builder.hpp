/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_PROTO_ACCOUNT_ASSET_BUILDER_HPP
#define IROHA_PROTO_ACCOUNT_ASSET_BUILDER_HPP

#include "backend/protobuf/common_objects/account_asset.hpp"
#include "qry_responses.pb.h"

namespace shared_model {
  namespace proto {
    /**
     * AccountAssetBuilder is used to construct AccountAsset proto objects with
     * initialized protobuf implementation
     */
    class DEPRECATED AccountAssetBuilder {
     public:
      shared_model::proto::AccountAsset build() {
        return shared_model::proto::AccountAsset(
            iroha::protocol::AccountAsset(account_asset_));
      }

      AccountAssetBuilder accountId(
          const interface::types::AccountIdType &account_id) {
        AccountAssetBuilder copy(*this);
        copy.account_asset_.set_account_id(account_id);
        return copy;
      }

      AccountAssetBuilder assetId(
          const interface::types::AssetIdType &asset_id) {
        AccountAssetBuilder copy(*this);
        copy.account_asset_.set_asset_id(asset_id);
        return copy;
      }

      AccountAssetBuilder balance(const interface::Amount &amount) {
        AccountAssetBuilder copy(*this);
        *copy.account_asset_.mutable_balance() = amount.toStringRepr();
        return copy;
      }

     private:
      iroha::protocol::AccountAsset account_asset_;
    };
  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_PROTO_ACCOUNT_ASSET_BUILDER_HPP

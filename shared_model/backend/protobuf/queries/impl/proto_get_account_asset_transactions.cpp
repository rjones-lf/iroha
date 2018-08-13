/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "backend/protobuf/queries/proto_get_account_asset_transactions.hpp"

namespace shared_model {
  namespace proto {

    template <typename QueryType>
    GetAccountAssetTransactions::GetAccountAssetTransactions(QueryType &&query)
        : CopyableProto(std::forward<QueryType>(query)),
          account_asset_transactions_{
              proto_->payload().get_account_asset_transactions()} {}

    template GetAccountAssetTransactions::GetAccountAssetTransactions(
        GetAccountAssetTransactions::TransportType &);
    template GetAccountAssetTransactions::GetAccountAssetTransactions(
        const GetAccountAssetTransactions::TransportType &);
    template GetAccountAssetTransactions::GetAccountAssetTransactions(
        GetAccountAssetTransactions::TransportType &&);

    GetAccountAssetTransactions::GetAccountAssetTransactions(
        const GetAccountAssetTransactions &o)
        : GetAccountAssetTransactions(o.proto_) {}

    GetAccountAssetTransactions::GetAccountAssetTransactions(
        GetAccountAssetTransactions &&o) noexcept
        : GetAccountAssetTransactions(std::move(o.proto_)) {}

    const interface::types::AccountIdType &
    GetAccountAssetTransactions::accountId() const {
      return account_asset_transactions_.account_id();
    }

    const interface::types::AssetIdType &GetAccountAssetTransactions::assetId()
        const {
      return account_asset_transactions_.asset_id();
    }

  }  // namespace proto
}  // namespace shared_model

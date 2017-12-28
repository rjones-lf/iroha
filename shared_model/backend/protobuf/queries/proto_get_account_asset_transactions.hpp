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

#ifndef IROHA_GET_ACCOUNT_ASSET_TRANSACTIONS_H
#define IROHA_GET_ACCOUNT_ASSET_TRANSACTIONS_H

#include "interfaces/queries/get_account_transactions.hpp"
#include "interfaces/common_objects/types.hpp"

#include "queries.pb.h"
#include "utils/lazy_initializer.hpp"
#include "utils/reference_holder.hpp"

namespace shared_model {
  namespace proto {
    class GetAccountAssetTransactions final
        : public CopyableProto<interface::GetAccountAssetTransactions,
                               iroha::protocol::Query,
                               GetAccountAssetTransactions> {
     public:
      template <typename QueryType>
      explicit GetAccountAssetTransactions(QueryType &&query)
          : CopyableProto(std::forward<QueryType>(query)),
            account_asset_transactions_(detail::makeReferenceGenerator(
                &proto_->payload(),
                &iroha::protocol::Query::Payload::
                    get_account_asset_transactions)),
            assets_id_([this] {
              return boost::accumulate(
                  account_asset_transactions_->assets_id(),
                  interface::types::AssetIdCollectionType{},
                  [](auto &&acc, const auto &asset) {
                    acc.emplace_back(asset);
                    return std::forward<decltype(acc)>(acc);
                  });
            }) {}

      GetAccountAssetTransactions(const GetAccountAssetTransactions &o)
          : GetAccountAssetTransactions(o.proto_) {}

      GetAccountAssetTransactions(GetAccountAssetTransactions &&o) noexcept
          : GetAccountAssetTransactions(std::move(o.proto_)) {}

      const interface::types::AccountIdType &accountId() const override {
        return account_asset_transactions_->account_id();
      }

      const interface::types::AssetIdCollectionType &assetsId() const override {
        return *assets_id_;
      }

     private:
      // ------------------------------| fields |-------------------------------

      template <typename T>
      using Lazy = detail::LazyInitializer<T>;

      const Lazy<const iroha::protocol::GetAccountAssetTransactions &>
          account_asset_transactions_;

      const Lazy<interface::types::AssetIdCollectionType> assets_id_;
    };

  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_GET_ACCOUNT_ASSET_TRANSACTIONS_H

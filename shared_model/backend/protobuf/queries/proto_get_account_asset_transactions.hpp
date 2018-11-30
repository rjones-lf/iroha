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

#include "backend/protobuf/common_objects/trivial_proto.hpp"
#include "backend/protobuf/queries/proto_tx_pagination_meta.hpp"
#include "interfaces/queries/get_account_asset_transactions.hpp"
#include "queries.pb.h"

namespace shared_model {
  namespace proto {
    class GetAccountAssetTransactions final
        : public CopyableProto<interface::GetAccountAssetTransactions,
                               iroha::protocol::Query,
                               GetAccountAssetTransactions> {
     public:
      template <typename QueryType>
      explicit GetAccountAssetTransactions(QueryType &&query);

      GetAccountAssetTransactions(const GetAccountAssetTransactions &o);

      GetAccountAssetTransactions(GetAccountAssetTransactions &&o) noexcept;

      const interface::types::AccountIdType &accountId() const override;

      const interface::types::AssetIdType &assetId() const override;

      const interface::TxPaginationMeta &paginationMeta() const override;

     private:
      // ------------------------------| fields |-------------------------------

      const iroha::protocol::GetAccountAssetTransactions
          &account_asset_transactions_;
      const TxPaginationMeta pagination_meta_;
    };

  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_GET_ACCOUNT_ASSET_TRANSACTIONS_H

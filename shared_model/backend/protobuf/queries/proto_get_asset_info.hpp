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

#ifndef IROHA_PROTO_GET_ASSET_INFO_H
#define IROHA_PROTO_GET_ASSET_INFO_H

#include "backend/protobuf/common_objects/trivial_proto.hpp"
#include "interfaces/queries/get_asset_info.hpp"
#include "queries.pb.h"

namespace shared_model {
  namespace proto {
    class GetAssetInfo final : public CopyableProto<interface::GetAssetInfo,
                                                    iroha::protocol::Query,
                                                    GetAssetInfo> {
     public:
      template <typename QueryType>
      explicit GetAssetInfo(QueryType &&query);

      GetAssetInfo(const GetAssetInfo &o);

      GetAssetInfo(GetAssetInfo &&o) noexcept;

      const interface::types::AssetIdType &assetId() const override;

     private:
      // ------------------------------| fields |-------------------------------
      const iroha::protocol::GetAssetInfo &asset_info_;
    };

  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_PROTO_GET_ASSET_INFO_H

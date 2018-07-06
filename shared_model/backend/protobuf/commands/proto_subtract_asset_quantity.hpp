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

#ifndef IROHA_PROTO_SUBTRACT_ASSET_QUANTITY_HPP
#define IROHA_PROTO_SUBTRACT_ASSET_QUANTITY_HPP

#include "interfaces/commands/subtract_asset_quantity.hpp"

#include "backend/protobuf/common_objects/amount.hpp"
#include "backend/protobuf/common_objects/trivial_proto.hpp"
#include "commands.pb.h"
#include "utils/lazy_initializer.hpp"

namespace shared_model {
  namespace proto {
    class SubtractAssetQuantity final
        : public CopyableProto<interface::SubtractAssetQuantity,
                               iroha::protocol::Command,
                               SubtractAssetQuantity> {
     public:
      template <typename CommandType>
      explicit SubtractAssetQuantity(CommandType &&command);

      SubtractAssetQuantity(const SubtractAssetQuantity &o);

      SubtractAssetQuantity(SubtractAssetQuantity &&o) noexcept;

      const interface::types::AssetIdType &assetId() const override;

      const interface::Amount &amount() const override;

     private:
      // lazy
      template <typename T>
      using Lazy = detail::LazyInitializer<T>;

      const iroha::protocol::SubtractAssetQuantity &subtract_asset_quantity_;

      const Lazy<proto::Amount> amount_;
    };

  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_PROTO_SUBTRACT_ASSET_QUANTITY_HPP

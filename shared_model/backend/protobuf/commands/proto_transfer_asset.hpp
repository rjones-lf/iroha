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

#ifndef IROHA_PROTO_TRANSFER_ASSET_HPP
#define IROHA_PROTO_TRANSFER_ASSET_HPP

#include "backend/protobuf/common_objects/trivial_proto.hpp"
#include "commands.pb.h"
#include "interfaces/commands/transfer_asset.hpp"
#include "interfaces/common_objects/amount.hpp"

namespace shared_model {
  namespace proto {

    class TransferAsset final : public CopyableProto<interface::TransferAsset,
                                                     iroha::protocol::Command,
                                                     TransferAsset> {
     public:
      template <typename CommandType>
      explicit TransferAsset(CommandType &&command);

      TransferAsset(const TransferAsset &o);

      TransferAsset(TransferAsset &&o) noexcept;

      const interface::Amount &amount() const override;

      const interface::types::AssetIdType &assetId() const override;

      const interface::types::AccountIdType &srcAccountId() const override;

      const interface::types::AccountIdType &destAccountId() const override;

      const interface::types::DescriptionType &description() const override;

     private:
      // lazy
      template <typename Value>
      using Lazy = detail::LazyInitializer<Value>;

      const iroha::protocol::TransferAsset &transfer_asset_;

      const Lazy<interface::Amount> amount_;
    };

  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_PROTO_TRANSFER_ASSET_HPP

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

#ifndef IROHA_SHARED_MODEL_PROTO_ASSET_HPP
#define IROHA_SHARED_MODEL_PROTO_ASSET_HPP

#include "backend/protobuf/common_objects/trivial_proto.hpp"
#include "interfaces/common_objects/asset.hpp"
#include "responses.pb.h"
#include "utils/lazy_initializer.hpp"
#include "utils/reference_holder.hpp"

namespace shared_model {
  namespace proto {
    class Asset final : public CopyableProto<interface::Asset,
                                             iroha::protocol::Asset,
                                             Asset> {
     public:
      template <typename AssetType>
      explicit Asset(AssetType &&account)
          : CopyableProto(std::forward<AssetType>(account)),
            assetId_(proto_->asset_id()),
            domainId_(proto_->domain_id()),
            precision_(proto_->precision()),
            blob_([this] { return makeBlob(*proto_); }) {}

      Asset(const Asset &o) : Asset(o.proto_) {}

      Asset(Asset &&o) noexcept : Asset(std::move(o.proto_)) {}

      const interface::types::AssetIdType &assetId() const override {
        return assetId_;
      }

      const interface::types::DomainIdType &domainId() const override {
        return domainId_;
      }

      const interface::types::PrecisionType &precision() const override {
        return precision_;
      }

      const BlobType &blob() const override { return *blob_; }

     private:
      template <typename T>
      using Lazy = detail::LazyInitializer<T>;

      interface::types::AssetIdType assetId_;

      interface::types::DomainIdType domainId_;

      interface::types::PrecisionType precision_;

      const Lazy<BlobType> blob_;
    };
  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_SHARED_MODEL_PROTO_ASSET_HPP

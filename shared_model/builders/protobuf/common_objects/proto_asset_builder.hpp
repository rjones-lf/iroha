/**
 * Copyright Soramitsu Co., Ltd. 2018 All Rights Reserved.
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

#ifndef IROHA_PROTO_ASSET_BUILDER_HPP
#define IROHA_PROTO_ASSET_BUILDER_HPP

#include "backend/protobuf/common_objects/asset.hpp"
#include "responses.pb.h"

namespace shared_model {
  namespace proto {

    /**
     * AssetBuilder is used to construct Asset proto objects with initialized
     * protobuf implementation
     */
    class AssetBuilder {
     public:
      shared_model::proto::Asset build() {
        return shared_model::proto::Asset(iroha::protocol::Asset(asset_));
      }

      AssetBuilder assetId(const interface::types::AssetIdType &asset_id) {
        AssetBuilder copy(*this);
        copy.asset_.set_asset_id(asset_id);
        return copy;
      }

      AssetBuilder domainId(const interface::types::DomainIdType &domain_id) {
        AssetBuilder copy(*this);
        copy.asset_.set_domain_id(domain_id);
        return copy;
      }

      AssetBuilder precision(const interface::types::PrecisionType &precision) {
        AssetBuilder copy(*this);
        copy.asset_.set_precision(precision);
        return copy;
      }

     private:
      iroha::protocol::Asset asset_;
    };
  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_PROTO_ASSET_BUILDER_HPP

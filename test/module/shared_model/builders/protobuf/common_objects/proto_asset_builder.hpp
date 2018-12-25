/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_PROTO_ASSET_BUILDER_HPP
#define IROHA_PROTO_ASSET_BUILDER_HPP

#include "backend/protobuf/common_objects/asset.hpp"
#include "qry_responses.pb.h"

namespace shared_model {
  namespace proto {

    /**
     * AssetBuilder is used to construct Asset proto objects with initialized
     * protobuf implementation
     */
    class DEPRECATED AssetBuilder {
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

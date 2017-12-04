//
// Created by Dumitru Savva on 04/12/2017.
//

#ifndef IROHA_PROTO_GET_ACCOUNT_ASSETS_H
#define IROHA_PROTO_GET_ACCOUNT_ASSETS_H

#include "interfaces/queries/get_account_assets.hpp"

#include "queries.pb.h"
#include "utils/lazy_initializer.hpp"
#include "utils/reference_holder.hpp"

namespace shared_model {
  namespace proto {
    class GetAccountAssets final
        : public CopyableProto<interface::GetAccountAssets,
                               iroha::protocol::Query,
                               GetAccountAssets> {
     public:
      template <typename QueryType>
      explicit GetAccountAssets(QueryType &&query)
          : CopyableProto(std::forward<QueryType>(query)),
            account_id_(detail::makeReferenceGenerator(
                &proto_->payload(),
                &iroha::protocol::Query::Payload::get_account)),
            asset_id_(detail::makeReferenceGenerator(
                &proto_->payload(),
                &iroha::protocol::Query::Payload::get_account_assets)) {}

      GetAccountAssets(const GetAccountAssets &o)
          : GetAccountAssets(o.proto_) {}

      GetAccountAssets(GetAccountAssets &&o) noexcept
          : GetAccountAssets(std::move(o.proto_)) {}

      const interface::types::AccountIdType &accountId() const override {
        return account_id_->account_id();
      }

      const interface::types::AssetIdType &assetId() const override {
        return asset_id_->asset_id();
      }

     private:
      // ------------------------------| fields |-------------------------------

      template <typename T>
      using Lazy = detail::LazyInitializer<T>;

      const Lazy<const iroha::protocol::GetAccount &> account_id_;
      const Lazy<const iroha::protocol::GetAccountAssets &> asset_id_;
    };

  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_PROTO_GET_ACCOUNT_ASSETS_H

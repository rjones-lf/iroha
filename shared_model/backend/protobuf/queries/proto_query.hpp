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

#ifndef IROHA_SHARED_MODEL_PROTO_QUERY_HPP
#define IROHA_SHARED_MODEL_PROTO_QUERY_HPP

#include <boost/range/numeric.hpp>

#include "backend/protobuf/common_objects/signature.hpp"
#include "backend/protobuf/common_objects/trivial_proto.hpp"
#include "interfaces/queries/query.hpp"
#include "queries.pb.h"
#include "utils/lazy_initializer.hpp"
#include "utils/variant_deserializer.hpp"

#include "backend/protobuf/queries/proto_get_account.hpp"
#include "backend/protobuf/queries/proto_get_account_assets.hpp"
#include "backend/protobuf/queries/proto_get_asset_info.hpp"
#include "backend/protobuf/queries/proto_get_roles.hpp"
#include "backend/protobuf/queries/proto_get_signatories.hpp"
#include "backend/protobuf/queries/proto_get_account_transactions.hpp"
#include "backend/protobuf/queries/proto_get_account_asset_transactions.hpp"

template <typename... T, typename Archive>
auto load_query(Archive &&ar) {
  int which = ar.payload()
                  .GetDescriptor()
                  ->FindFieldByNumber(ar.payload().query_case())
                  ->index_in_oneof();
  return shared_model::detail::variant_impl<T...>::template load<
      shared_model::interface::Query::QueryVariantType>(
      std::forward<Archive>(ar), which);
}

namespace shared_model {
  namespace proto {
    class Query final : public CopyableProto<interface::Query,
                                             iroha::protocol::Query,
                                             Query> {
     private:
      /// polymorphic wrapper type shortcut
      template <typename... Value>
      using wrap = boost::variant<detail::PolymorphicWrapper<Value>...>;

      /// lazy variant shortcut
      template <typename T>
      using Lazy = detail::LazyInitializer<T>;

      using LazyVariantType = Lazy<QueryVariantType>;

     public:
      /// type of proto variant
      using ProtoQueryVariantType = wrap<GetAccount,
                                         GetSignatories,
                                         GetAccountTransactions,
                                         GetAccountAssetTransactions,
                                         GetAccountAssets,
                                         GetRoles,
                                         GetRolePermissions,
                                         GetAssetInfo>;

      /// list of types in proto variant
      using ProtoQueryListType = ProtoQueryVariantType::types;

      template <typename QueryType>
      explicit Query(QueryType &&query)
          : CopyableProto(std::forward<QueryType>(query)),
            variant_(
                [this] { return load_query<ProtoQueryListType>(*proto_); }),
            blob_([this] { return BlobType(proto_->SerializeAsString()); }),
            signatures_([this] {
              SignatureSetType set;
              set.emplace(new Signature(proto_->signature()));
              return set;
            }) {}

      Query(const Query &o) : Query(o.proto_) {}

      Query(Query &&o) noexcept : Query(std::move(o.proto_)) {}

      const QueryVariantType &get() const override { return *variant_; }

      const interface::types::AccountIdType &creatorAccountId() const override {
        return proto_->payload().creator_account_id();
      }

      QueryCounterType queryCounter() const override {
        return proto_->payload().query_counter();
      }

      const BlobType &blob() const override { return *blob_; }

      // ------------------------| Signable override  |-------------------------
      const SignatureSetType &signatures() const override {
        return *signatures_;
      }

      bool addSignature(const SignatureType &signature) override {
        if (proto_->has_signature()) return false;

        auto sig = proto_->mutable_signature();
        sig->set_pubkey(signature->publicKey().blob());
        sig->set_signature(signature->signedData().blob());
        return true;
      }

      TimestampType createdTime() const override {
        return proto_->payload().created_time();
      }

     private:
      // ------------------------------| fields |-------------------------------
      // lazy
      const LazyVariantType variant_;

      const Lazy<BlobType> blob_;
      const Lazy<SignatureSetType> signatures_;
    };
  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_SHARED_MODEL_PROTO_COMMAND_HPP

/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_SHARED_MODEL_PROTO_QUERY_HPP
#define IROHA_SHARED_MODEL_PROTO_QUERY_HPP

#include "interfaces/queries/query.hpp"

#include "backend/protobuf/common_objects/signature.hpp"
#include "backend/protobuf/common_objects/trivial_proto.hpp"
#include "backend/protobuf/queries/proto_get_account.hpp"
#include "backend/protobuf/queries/proto_get_account_asset_transactions.hpp"
#include "backend/protobuf/queries/proto_get_account_assets.hpp"
#include "backend/protobuf/queries/proto_get_account_detail.hpp"
#include "backend/protobuf/queries/proto_get_account_transactions.hpp"
#include "backend/protobuf/queries/proto_get_asset_info.hpp"
#include "backend/protobuf/queries/proto_get_role_permissions.hpp"
#include "backend/protobuf/queries/proto_get_roles.hpp"
#include "backend/protobuf/queries/proto_get_signatories.hpp"
#include "backend/protobuf/queries/proto_get_transactions.hpp"
#include "backend/protobuf/queries/proto_get_pending_transactions.hpp"
#include "queries.pb.h"
#include "utils/lazy_initializer.hpp"

namespace shared_model {
  namespace proto {
    class Query FINAL : public CopyableProto<interface::Query,
                                             iroha::protocol::Query,
                                             Query> {
     public:
      /// type of proto variant
      using ProtoQueryVariantType = boost::variant<GetAccount,
                                                   GetSignatories,
                                                   GetAccountTransactions,
                                                   GetAccountAssetTransactions,
                                                   GetTransactions,
                                                   GetAccountAssets,
                                                   GetAccountDetail,
                                                   GetRoles,
                                                   GetRolePermissions,
                                                   GetAssetInfo,
                                                   GetPendingTransactions>;

      /// list of types in proto variant
      using ProtoQueryListType = ProtoQueryVariantType::types;

      template <typename QueryType>
      explicit Query(QueryType &&query);

      Query(const Query &o);

      Query(Query &&o) noexcept;

      const Query::QueryVariantType &get() const override;

      const interface::types::AccountIdType &creatorAccountId() const override;

      interface::types::CounterType queryCounter() const override;

      const interface::types::BlobType &blob() const override;

      const interface::types::BlobType &payload() const override;

      // ------------------------| Signable override  |-------------------------
      interface::types::SignatureRangeType signatures() const override;

      bool addSignature(const crypto::Signed &signed_blob,
                        const crypto::PublicKey &public_key) override;

      interface::types::TimestampType createdTime() const override;

     private:
      /// lazy variant shortcut
      template <typename T>
      using Lazy = detail::LazyInitializer<T>;

      using LazyVariantType = Lazy<ProtoQueryVariantType>;
      // ------------------------------| fields |-------------------------------
      // lazy
      const LazyVariantType variant_;

      const Lazy<QueryVariantType> ivariant_;

      const Lazy<interface::types::BlobType> blob_;

      const Lazy<interface::types::BlobType> payload_;

      const Lazy<SignatureSetType<proto::Signature>> signatures_;
    };
  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_SHARED_MODEL_PROTO_QUERY_HPP

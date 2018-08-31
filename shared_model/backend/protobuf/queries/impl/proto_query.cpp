/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "backend/protobuf/queries/proto_query.hpp"
#include "backend/protobuf/util.hpp"
#include "utils/variant_deserializer.hpp"

namespace shared_model {
  namespace proto {

    template <typename QueryType>
    Query::Query(QueryType &&query)
        : CopyableProto(std::forward<QueryType>(query)),
          variant_{[this] {
            auto &&ar = *proto_;
            int which = ar.payload()
                            .GetDescriptor()
                            ->FindFieldByNumber(ar.payload().query_case())
                            ->index_in_oneof();
            return shared_model::detail::
                variant_impl<Query::ProtoQueryListType>::template load<
                    Query::ProtoQueryVariantType>(
                    std::forward<decltype(ar)>(ar), which);
          }},
          ivariant_{detail::makeLazyInitializer(
              [this] { return QueryVariantType(*Query::variant_); })},
          blob_{[this] { return makeBlob(*proto_); }},
          payload_{[this] { return makeBlob(proto_->payload()); }},
          signatures_{[this] {
            SignatureSetType<proto::Signature> set;
            if (proto_->has_signature()) {
              set.emplace(proto_->signature());
            }
            return set;
          }} {}

    template Query::Query(Query::TransportType &);
    template Query::Query(const Query::TransportType &);
    template Query::Query(Query::TransportType &&);

    Query::Query(const Query &o) : Query(o.proto_) {}

    Query::Query(Query &&o) noexcept : Query(std::move(o.proto_)) {}

    const Query::QueryVariantType &Query::get() const {
      return *ivariant_;
    }

    const interface::types::AccountIdType &Query::creatorAccountId() const {
      return proto_->payload().meta().creator_account_id();
    }

    interface::types::CounterType Query::queryCounter() const {
      return proto_->payload().meta().query_counter();
    }

    const interface::types::BlobType &Query::blob() const {
      return *blob_;
    }

    const interface::types::BlobType &Query::payload() const {
      return *payload_;
    }

    interface::types::SignatureRangeType Query::signatures() const {
      return *signatures_;
    }

    bool Query::addSignature(const crypto::Signed &signed_blob,
                             const crypto::PublicKey &public_key) {
      if (proto_->has_signature()) {
        return false;
      }

      auto sig = proto_->mutable_signature();
      sig->set_signature(crypto::toBinaryString(signed_blob));
      sig->set_public_key(crypto::toBinaryString(public_key));
      return true;
    }

    interface::types::TimestampType Query::createdTime() const {
      return proto_->payload().meta().created_time();
    }

  }  // namespace proto
}  // namespace shared_model

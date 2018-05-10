/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_SHARED_MODEL_PROTO_QUERY_PAYLOAD_META_HPP
#define IROHA_SHARED_MODEL_PROTO_QUERY_PAYLOAD_META_HPP

#include "interfaces/queries/query_payload_meta.hpp"
#include "queries.pb.h"

namespace shared_model {
  namespace proto {
    class QueryPayloadMeta FINAL
        : public CopyableProto<interface::QueryPayloadMeta,
                               iroha::protocol::QueryPayloadMeta,
                               QueryPayloadMeta> {
     public:
      template <typename QueryPayloadMetaType>
      explicit QueryPayloadMeta(QueryPayloadMetaType &&query)
          : CopyableProto(std::forward<QueryPayloadMetaType>(query)) {}

      QueryPayloadMeta(const QueryPayloadMeta &o)
          : QueryPayloadMeta(o.proto_) {}

      QueryPayloadMeta(QueryPayloadMeta &&o) noexcept
          : QueryPayloadMeta(std::move(o.proto_)) {}

      const interface::types::AccountIdType &creatorAccountId() const override {
        return proto_->creator_account_id();
      }

      interface::types::CounterType queryCounter() const override {
        return proto_->query_counter();
      }

      interface::types::TimestampType createdTime() const override {
        return proto_->created_time();
      }
    };
  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_SHARED_MODEL_PROTO_QUERY_HPP

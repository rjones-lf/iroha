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

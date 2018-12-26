/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_SHARED_MODEL_PROTO_BLOCKS_QUERY_HPP
#define IROHA_SHARED_MODEL_PROTO_BLOCKS_QUERY_HPP

#include "backend/protobuf/common_objects/signature.hpp"
#include "interfaces/queries/blocks_query.hpp"
#include "queries.pb.h"
#include "backend/protobuf/util.hpp"

namespace shared_model {
  namespace proto {
    class BlocksQuery FINAL : public CopyableProto<interface::BlocksQuery,
                                                   iroha::protocol::BlocksQuery,
                                                   BlocksQuery> {
     public:
      template <typename BlocksQueryType>
      explicit BlocksQuery(BlocksQueryType &&query);

      BlocksQuery(const BlocksQuery &o);

      BlocksQuery(BlocksQuery &&o) noexcept;

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
      // ------------------------------| fields |-------------------------------
      const interface::types::BlobType blob_;

      const interface::types::BlobType payload_;

      SignatureSetType<proto::Signature> signatures_;
    };
  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_SHARED_MODEL_PROTO_BLOCKS_QUERY_HPP

/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_SHARED_MODEL_PROTO_PROPOSAL_HPP
#define IROHA_SHARED_MODEL_PROTO_PROPOSAL_HPP

#include "backend/protobuf/transaction.hpp"
#include "interfaces/iroha_internal/proposal.hpp"

#include "common_objects/noncopyable_proto.hpp"

#include "interfaces/common_objects/types.hpp"
#include "proposal.pb.h"
#include "utils/lazy_initializer.hpp"

namespace shared_model {
  namespace proto {
    class Proposal final : public NonCopyableProto<interface::Proposal,
                                                   iroha::protocol::Proposal,
                                                   Proposal> {
     public:
      using NonCopyableProto::NonCopyableProto;

      Proposal(Proposal &&o) noexcept;
      Proposal &operator=(Proposal &&o) noexcept;

      interface::types::TransactionsCollectionType transactions()
          const override;

      interface::types::TimestampType createdTime() const override;

      interface::types::HeightType height() const override;

      const interface::types::BlobType &blob() const override;

     private:
      template <typename T>
      using Lazy = detail::LazyInitializer<T>;

      const Lazy<std::vector<proto::Transaction>> transactions_{[this] {
        return std::vector<proto::Transaction>(
            proto_.mutable_transactions()->begin(),
            proto_.mutable_transactions()->end());
      }};

      Lazy<interface::types::BlobType> blob_{
          [this] { return makeBlob(proto_); }};
    };
  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_PROPOSAL_HPP

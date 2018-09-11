/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "backend/protobuf/proposal.hpp"

namespace shared_model {
  namespace proto {
    using namespace interface::types;

    Proposal::Proposal(Proposal &&o) noexcept
        : NonCopyableProto(std::move(o.proto_)) {}

    Proposal &Proposal::operator=(Proposal &&o) noexcept {
      proto_ = std::move(o.proto_);

      hash_.invalidate();
      transactions_.invalidate();
      blob_.invalidate();

      return *this;
    }

    TransactionsCollectionType Proposal::transactions() const {
      return *transactions_;
    }

    TimestampType Proposal::createdTime() const {
      return proto_.created_time();
    }

    HeightType Proposal::height() const {
      return proto_.height();
    }

    const interface::types::BlobType &Proposal::blob() const {
      return *blob_;
    }

  }  // namespace proto
}  // namespace shared_model

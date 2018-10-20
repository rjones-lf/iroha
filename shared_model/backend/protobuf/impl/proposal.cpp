/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "backend/protobuf/proposal.hpp"

#include "backend/protobuf/transaction.hpp"

namespace shared_model {
  namespace proto {
    using namespace interface::types;

    struct Proposal::Impl {
      Impl(TransportType &&ref) : proto_(std::move(ref)) {}
      Impl(const TransportType &ref) : proto_(ref) {}
      Impl(Impl &&o) noexcept = delete;
      Impl &operator=(Impl &&o) noexcept = delete;
      TransportType proto_;
      const std::vector<proto::Transaction> transactions_{[this] {
        return std::vector<proto::Transaction>(
            proto_.mutable_transactions()->begin(),
            proto_.mutable_transactions()->end());
      }()};

      interface::types::BlobType blob_{[this] { return makeBlob(proto_); }()};
    };

    Proposal::Proposal(Proposal &&o) noexcept : Proposal(o.getTransport()) {}

    Proposal::Proposal(TransportType ref) {
      impl_ = std::make_unique<Proposal::Impl>(ref);
    }

    TransactionsCollectionType Proposal::transactions() const {
      return impl_->transactions_;
    }

    TimestampType Proposal::createdTime() const {
      return impl_->proto_.created_time();
    }

    HeightType Proposal::height() const {
      return impl_->proto_.height();
    }

    const interface::types::BlobType &Proposal::blob() const {
      return impl_->blob_;
    }

    const Proposal::TransportType &Proposal::getTransport() const {
      return impl_->proto_;
    }

    Proposal::ModelType *Proposal::clone() const {
      return new Proposal(impl_->proto_);
    }

    Proposal::~Proposal() = default;

  }  // namespace proto
}  // namespace shared_model

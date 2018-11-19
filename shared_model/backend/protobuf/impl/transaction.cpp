/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "backend/protobuf/transaction.hpp"

#include <boost/range/adaptor/transformed.hpp>
#include "backend/protobuf/batch_meta.hpp"
#include "backend/protobuf/commands/proto_command.hpp"
#include "backend/protobuf/common_objects/signature.hpp"
#include "backend/protobuf/util.hpp"
#include "utils/reference_holder.hpp"

namespace shared_model {
  namespace proto {

    struct Transaction::Impl {
      explicit Impl(TransportType &&ref) : proto_{std::move(ref)} {}

      explicit Impl(const TransportType &ref) : proto_{ref} {}

      detail::ReferenceHolder<TransportType> proto_;

      iroha::protocol::Transaction::Payload &payload_{
          *proto_->mutable_payload()};

      iroha::protocol::Transaction::Payload::ReducedPayload &reduced_payload_{
          *proto_->mutable_payload()->mutable_reduced_payload()};

      interface::types::BlobType blob_{[this] { return makeBlob(*proto_); }()};

      interface::types::BlobType payload_blob_{
          [this] { return makeBlob(payload_); }()};

      interface::types::BlobType reduced_payload_blob_{
          [this] { return makeBlob(reduced_payload_); }()};

      interface::types::HashType reduced_hash_{
          shared_model::crypto::Sha3_256::makeHash(reduced_payload_blob_)};

      std::vector<proto::Command> commands_{[this] {
        return std::vector<proto::Command>(reduced_payload_.commands().begin(),
                                           reduced_payload_.commands().end());
      }()};

      boost::optional<std::shared_ptr<interface::BatchMeta>> meta_{
          [this]() -> boost::optional<std::shared_ptr<interface::BatchMeta>> {
            if (payload_.has_batch()) {
              std::shared_ptr<interface::BatchMeta> b =
                  std::make_shared<proto::BatchMeta>(payload_.batch());
              return b;
            }
            return boost::none;
          }()};

      SignatureSetType<proto::Signature> signatures_{[this] {
        auto signatures = proto_->signatures()
            | boost::adaptors::transformed([](const auto &x) {
                            return proto::Signature(x);
                          });
        return SignatureSetType<proto::Signature>(signatures.begin(),
                                                  signatures.end());
      }()};
    };

    Transaction::Transaction(const TransportType &transaction) {
      impl_ = std::make_unique<Transaction::Impl>(transaction);
    }

    Transaction::Transaction(TransportType &&transaction) {
      impl_ = std::make_unique<Transaction::Impl>(std::move(transaction));
    }

    // TODO [IR-1866] Akvinikym 13.11.18: remove the copy ctor and fix fallen
    // tests
    Transaction::Transaction(const Transaction &transaction)
        : Transaction(*transaction.impl_->proto_) {}

    Transaction::Transaction(Transaction &&transaction) noexcept = default;

    Transaction::~Transaction() = default;

    const interface::types::AccountIdType &Transaction::creatorAccountId()
        const {
      return impl_->reduced_payload_.creator_account_id();
    }

    Transaction::CommandsType Transaction::commands() const {
      return impl_->commands_;
    }

    const interface::types::BlobType &Transaction::blob() const {
      return impl_->blob_;
    }

    const interface::types::BlobType &Transaction::payload() const {
      return impl_->payload_blob_;
    }

    const interface::types::BlobType &Transaction::reducedPayload() const {
      return impl_->reduced_payload_blob_;
    }

    interface::types::SignatureRangeType Transaction::signatures() const {
      return impl_->signatures_;
    }

    const interface::types::HashType &Transaction::reducedHash() const {
      return impl_->reduced_hash_;
    }

    bool Transaction::addSignature(const crypto::Signed &signed_blob,
                                   const crypto::PublicKey &public_key) {
      // if already has such signature
      if (std::find_if(impl_->signatures_.begin(),
                       impl_->signatures_.end(),
                       [&public_key](const auto &signature) {
                         return signature.publicKey() == public_key;
                       })
          != impl_->signatures_.end()) {
        return false;
      }

      auto sig = impl_->proto_->add_signatures();
      sig->set_signature(crypto::toBinaryString(signed_blob));
      sig->set_public_key(crypto::toBinaryString(public_key));

      impl_->signatures_ = [this] {
        auto signatures = impl_->proto_->signatures()
            | boost::adaptors::transformed([](const auto &x) {
                            return proto::Signature(x);
                          });
        return SignatureSetType<proto::Signature>(signatures.begin(),
                                                  signatures.end());
      }();

      return true;
    }

    const Transaction::TransportType &Transaction::getTransport() const {
      return *impl_->proto_;
    }

    interface::types::TimestampType Transaction::createdTime() const {
      return impl_->reduced_payload_.created_time();
    }

    interface::types::QuorumType Transaction::quorum() const {
      return impl_->reduced_payload_.quorum();
    }

    boost::optional<std::shared_ptr<interface::BatchMeta>>
    Transaction::batchMeta() const {
      return impl_->meta_;
    }

    Transaction::ModelType *Transaction::clone() const {
      return new Transaction(*impl_->proto_);
    }

  }  // namespace proto
}  // namespace shared_model

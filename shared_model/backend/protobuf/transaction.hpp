/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_SHARED_MODEL_PROTO_TRANSACTION_HPP
#define IROHA_SHARED_MODEL_PROTO_TRANSACTION_HPP

#include "interfaces/transaction.hpp"

#include <boost/range/adaptor/transformed.hpp>

#include "backend/protobuf/commands/proto_command.hpp"
#include "backend/protobuf/common_objects/signature.hpp"
#include "batch_meta.hpp"
#include "utils/lazy_initializer.hpp"

namespace shared_model {
  namespace proto {
    class Transaction FINAL : public CopyableProto<interface::Transaction,
                                                   iroha::protocol::Transaction,
                                                   Transaction> {
     public:
      template <typename TransactionType>
      explicit Transaction(TransactionType &&transaction)
          : CopyableProto(std::forward<TransactionType>(transaction)) {}

      Transaction(const Transaction &o) : Transaction(o.proto_) {}

      Transaction(Transaction &&o) noexcept
          : Transaction(std::move(o.proto_)) {}

      const interface::types::AccountIdType &creatorAccountId() const override {
        return reduced_payload_.creator_account_id();
      }

      Transaction::CommandsType commands() const override {
        return *commands_;
      }

      const interface::types::BlobType &blob() const override {
        return *blob_;
      }

      const interface::types::BlobType &payload() const override {
        return *blobTypePayload_;
      }

      const interface::types::BlobType &reducedPayload() const override {
        return *blobTypeReducedPayload_;
      }

      interface::types::SignatureRangeType signatures() const override {
        return *signatures_;
      }

      const interface::types::HashType &reducedHash() const override {
        if (reduced_hash_ == boost::none) {
          reduced_hash_.emplace(HashProvider::makeHash(reducedPayload()));
        }
        return *reduced_hash_;
      }

      bool addSignature(const crypto::Signed &signed_blob,
                        const crypto::PublicKey &public_key) override {
        // if already has such signature
        if (std::find_if(signatures_->begin(),
                         signatures_->end(),
                         [&public_key](const auto &signature) {
                           return signature.publicKey() == public_key;
                         })
            != signatures_->end()) {
          return false;
        }

        auto sig = proto_->add_signatures();
        sig->set_signature(crypto::toBinaryString(signed_blob));
        sig->set_public_key(crypto::toBinaryString(public_key));

        signatures_.invalidate();
        return true;
      }

      interface::types::TimestampType createdTime() const override {
        return reduced_payload_.created_time();
      }

      interface::types::QuorumType quorum() const override {
        return reduced_payload_.quorum();
      }

      boost::optional<std::shared_ptr<interface::BatchMeta>> batchMeta()
          const override {
        return *meta_;
      }

     private:
      using HashProvider = shared_model::crypto::Sha3_256;
      mutable boost::optional<interface::types::HashType> reduced_hash_;
      // lazy
      template <typename T>
      using Lazy = detail::LazyInitializer<T>;

      const iroha::protocol::Transaction::Payload &payload_{proto_->payload()};

      const iroha::protocol::Transaction::Payload::ReducedPayload
          reduced_payload_{proto_->payload().reduced_payload()};

      const Lazy<std::vector<proto::Command>> commands_{[this] {
        return std::vector<proto::Command>(reduced_payload_.commands().begin(),
                                           reduced_payload_.commands().end());
      }};

      const Lazy<interface::types::BlobType> blob_{
          [this] { return makeBlob(*proto_); }};

      const Lazy<interface::types::BlobType> blobTypePayload_{
          [this] { return makeBlob(payload_); }};

      const Lazy<interface::types::BlobType> blobTypeReducedPayload_{
          [this] { return makeBlob(reduced_payload_); }};

      const Lazy<boost::optional<std::shared_ptr<interface::BatchMeta>>> meta_{
          [this]() -> boost::optional<std::shared_ptr<interface::BatchMeta>> {
            if (payload_.has_batch()) {
              std::shared_ptr<interface::BatchMeta> b =
                  std::make_shared<proto::BatchMeta>(payload_.batch());
              return b;
            }
            return boost::none;
          }};

      const Lazy<SignatureSetType<proto::Signature>> signatures_{[this] {
        auto signatures = proto_->signatures()
            | boost::adaptors::transformed([](const auto &x) {
                            return proto::Signature(x);
                          });
        return SignatureSetType<proto::Signature>(signatures.begin(),
                                                  signatures.end());
      }};
    };
  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_SHARED_MODEL_PROTO_TRANSACTION_HPP

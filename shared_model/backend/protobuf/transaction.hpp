/**
 * Copyright Soramitsu Co., Ltd. 2017 All Rights Reserved.
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

#ifndef IROHA_SHARED_MODEL_PROTO_TRANSACTION_HPP
#define IROHA_SHARED_MODEL_PROTO_TRANSACTION_HPP

#include "interfaces/transaction.hpp"

#include <boost/range/adaptor/transformed.hpp>

#include "backend/protobuf/commands/proto_command.hpp"
#include "backend/protobuf/common_objects/signature.hpp"
#include "batch_meta.hpp"
#include "block.pb.h"
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

      const interface::types::BlobType &reduced_payload() const override {
        return *blobTypeReducedPayload_;
      }

      interface::types::SignatureRangeType signatures() const override {
        return *signatures_;
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
        sig->set_pubkey(crypto::toBinaryString(public_key));

        signatures_.invalidate();
        return true;
      }

      interface::types::TimestampType createdTime() const override {
        return reduced_payload_.created_time();
      }

      interface::types::QuorumType quorum() const override {
        return reduced_payload_.quorum();
      }

      boost::optional<std::shared_ptr<interface::BatchMeta>> batch_meta()
          const override {
        return *meta_;
      }

     private:
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
          [this] () -> boost::optional<std::shared_ptr<interface::BatchMeta>> {
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

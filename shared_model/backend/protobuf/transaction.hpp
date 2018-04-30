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

#include <boost/range/numeric.hpp>

#include "backend/protobuf/commands/proto_command.hpp"
#include "backend/protobuf/common_objects/signature.hpp"
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
        return payload_.creator_account_id();
      }

      const Transaction::CommandsType &commands() const override {
        return *commands_;
      }

      const interface::types::BlobType &blob() const override {
        return *blob_;
      }

      const interface::types::BlobType &payload() const override {
        return *blobTypePayload_;
      }

      const interface::SignatureSetType &signatures() const override {
        return *signatures_;
      }

      bool addSignature(const crypto::Signed &signed_blob,
                        const crypto::PublicKey &public_key) override {
        // if already has such signature
        if (std::find_if(signatures_->begin(),
                         signatures_->end(),
                         [&signed_blob, &public_key](auto signature) {
                           return signature->signedData() == signed_blob
                               and signature->publicKey() == public_key;
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

      bool clearSignatures() override {
        signatures_->clear();
        return (signatures_->size() == 0);
      }

      interface::types::TimestampType createdTime() const override {
        return payload_.created_time();
      }

     private:
      // lazy
      template <typename T>
      using Lazy = detail::LazyInitializer<T>;

      const iroha::protocol::Transaction::Payload &payload_{proto_->payload()};

      const Lazy<CommandsType> commands_{[this] {
        return boost::accumulate(payload_.commands(),
                                 CommandsType{},
                                 [](auto &&acc, const auto &cmd) {
                                   acc.emplace_back(new Command(cmd));
                                   return std::forward<decltype(acc)>(acc);
                                 });
      }};

      const Lazy<interface::types::BlobType> blob_{
          [this] { return makeBlob(*proto_); }};

      const Lazy<interface::types::BlobType> blobTypePayload_{
          [this] { return makeBlob(payload_); }};

      const Lazy<interface::SignatureSetType> signatures_{[this] {
        return boost::accumulate(proto_->signatures(),
                                 interface::SignatureSetType{},
                                 [](auto &&acc, const auto &sig) {
                                   acc.emplace(new Signature(sig));
                                   return std::forward<decltype(acc)>(acc);
                                 });
      }};
    };
  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_SHARED_MODEL_PROTO_TRANSACTION_HPP

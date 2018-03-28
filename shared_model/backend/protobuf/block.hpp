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

#ifndef IROHA_SHARED_MODEL_PROTO_BLOCK_HPP
#define IROHA_SHARED_MODEL_PROTO_BLOCK_HPP

#include "interfaces/iroha_internal/block.hpp"

#include <boost/range/numeric.hpp>
#include "backend/protobuf/common_objects/signature.hpp"
#include "backend/protobuf/transaction.hpp"
#include "backend/protobuf/util.hpp"
#include "common_objects/trivial_proto.hpp"
#include "interfaces/common_objects/types.hpp"

#include "block.pb.h"
#include "utils/lazy_initializer.hpp"

namespace shared_model {
  namespace proto {
    class Block final : public CopyableProto<interface::Block,
                                             iroha::protocol::Block,
                                             Block> {
      template <class T>
      using w = detail::PolymorphicWrapper<T>;

     public:
      template <class BlockType>
      explicit Block(BlockType &&block)
          : CopyableProto(std::forward<BlockType>(block)) {}

      Block(const Block &o) : Block(o.proto_) {}

      Block(Block &&o) noexcept : Block(std::move(o.proto_)) {}

      const interface::types::TransactionsCollectionType &transactions()
          const override {
        return *transactions_;
      }

      interface::types::HeightType height() const override {
        return payload_.height();
      }

      const interface::types::HashType &prevHash() const override {
        return *prev_hash_;
      }

      const interface::types::BlobType &blob() const override {
        return *blob_;
      }

      const interface::SignatureSetType &signatures() const override {
        return *signatures_;
      }

      // TODO Alexey Chernyshov - 2018-03-28 -
      // rework code duplication after fix protobuf
      // https://soramitsu.atlassian.net/browse/IR-1175
      bool addSignature(const crypto::Signed &signed_blob,
                        const crypto::PublicKey &public_key) override {
        // if already has such signature
        if (std::find_if(signatures_->begin(),
                     signatures_->end(),
                     [&signed_blob, &public_key](auto signature) {
                       return signature->signedData() == signed_blob
                           and signature->publicKey() == public_key;
                     }) != signatures_->end()) {
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

      interface::types::TransactionsNumberType txsNumber() const override {
        return payload_.tx_number();
      }

      const interface::types::BlobType &payload() const override {
        return *payload_blob_;
      }

     private:
      // lazy
      template <typename T>
      using Lazy = detail::LazyInitializer<T>;

      const iroha::protocol::Block::Payload &payload_{proto_->payload()};

      const Lazy<std::vector<w<interface::Transaction>>> transactions_{[this] {
        std::vector<w<interface::Transaction>> txs;
        for (const auto &tx : payload_.transactions()) {
          auto tmp = detail::makePolymorphic<proto::Transaction>(tx);
          txs.emplace_back(tmp);
        }
        return txs;
      }};

      const Lazy<interface::types::BlobType> blob_{
          [this] { return makeBlob(*proto_); }};

      const Lazy<interface::types::HashType> prev_hash_{[this] {
        return interface::types::HashType(proto_->payload().prev_block_hash());
      }};

      const Lazy<interface::SignatureSetType> signatures_{[this] {
        interface::SignatureSetType sigs;
        for (const auto &sig : proto_->signatures()) {
          auto curr = detail::makePolymorphic<proto::Signature>(sig);
          sigs.insert(curr);
        }
        return sigs;
      }};

      const Lazy<interface::types::BlobType> payload_blob_{
          [this] { return makeBlob(payload_); }};
    };
  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_SHARED_MODEL_PROTO_BLOCK_HPP

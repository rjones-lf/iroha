/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_SHARED_MODEL_PROTO_EMPTY_BLOCK_HPP
#define IROHA_SHARED_MODEL_PROTO_EMPTY_BLOCK_HPP

#include "interfaces/iroha_internal/empty_block.hpp"

#include "backend/protobuf/common_objects/signature.hpp"
#include "backend/protobuf/util.hpp"
#include "block.pb.h"
#include "common_objects/trivial_proto.hpp"
#include "interfaces/common_objects/types.hpp"
#include "utils/lazy_initializer.hpp"

namespace shared_model {
  namespace proto {
    class EmptyBlock final : public CopyableProto<interface::EmptyBlock,
                                                  iroha::protocol::Block,
                                                  EmptyBlock> {
     public:
      template <class BlockType>
      explicit EmptyBlock(BlockType &&block)
          : CopyableProto(std::forward<BlockType>(block)) {}

      EmptyBlock(const EmptyBlock &o) : EmptyBlock(o.proto_) {}

      EmptyBlock(EmptyBlock &&o) noexcept : EmptyBlock(std::move(o.proto_)) {}

      interface::types::HeightType height() const override {
        return payload_.height();
      }

      const interface::types::HashType &prevHash() const override {
        return *prev_hash_;
      }

      const interface::types::BlobType &blob() const override {
        return *blob_;
      }

      interface::types::SignatureRangeType signatures() const override {
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
        return payload_.created_time();
      }

      const interface::types::BlobType &payload() const override {
        return *payload_blob_;
      }

     private:
      template <typename T>
      using Lazy = detail::LazyInitializer<T>;

      const iroha::protocol::Block::Payload &payload_{proto_->payload()};

      const Lazy<interface::types::BlobType> blob_{
          [this] { return makeBlob(*proto_); }};

      const Lazy<interface::types::HashType> prev_hash_{[this] {
        return interface::types::HashType(proto_->payload().prev_block_hash());
      }};

      const Lazy<SignatureSetType<proto::Signature>> signatures_{[this] {
        SignatureSetType<proto::Signature> sigs;
        for (const auto &sig : proto_->signatures()) {
          sigs.emplace(sig);
        }
        return sigs;
      }};

      const Lazy<interface::types::BlobType> payload_blob_{
          [this] { return makeBlob(payload_); }};
    };
  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_SHARED_MODEL_PROTO_EMPTY_BLOCK_HPP

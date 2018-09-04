/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "backend/protobuf/block.hpp"

namespace shared_model {
  namespace proto {
    Block::Block(Block &&o) noexcept : NonCopyableProto(std::move(o.proto_)) {}

    Block &Block::operator=(Block &&o) noexcept {
      proto_ = std::move(o.proto_);
      payload_ = *proto_.mutable_payload();

      transactions_.invalidate();
      blob_.invalidate();
      prev_hash_.invalidate();
      signatures_.invalidate();
      payload_blob_.invalidate();

      return *this;
    }

    interface::types::TransactionsCollectionType Block::transactions() const {
      return *transactions_;
    }

    interface::types::HeightType Block::height() const {
      return payload_.height();
    }

    const interface::types::HashType &Block::prevHash() const {
      return *prev_hash_;
    }

    const interface::types::BlobType &Block::blob() const {
      return *blob_;
    }

    interface::types::SignatureRangeType Block::signatures() const {
      return *signatures_;
    }

    // TODO Alexey Chernyshov - 2018-03-28 -
    // rework code duplication from transaction, block after fix protobuf
    // https://soramitsu.atlassian.net/browse/IR-1175
    bool Block::addSignature(const crypto::Signed &signed_blob,
                             const crypto::PublicKey &public_key) {
      // if already has such signature
      if (std::find_if(signatures_->begin(),
                       signatures_->end(),
                       [&public_key](const auto &signature) {
                         return signature.publicKey() == public_key;
                       })
          != signatures_->end()) {
        return false;
      }

      auto sig = proto_.add_signatures();
      sig->set_signature(crypto::toBinaryString(signed_blob));
      sig->set_public_key(crypto::toBinaryString(public_key));

      signatures_.invalidate();
      return true;
    }

    interface::types::TimestampType Block::createdTime() const {
      return payload_.created_time();
    }

    interface::types::TransactionsNumberType Block::txsNumber() const {
      return payload_.tx_number();
    }

    const interface::types::BlobType &Block::payload() const {
      return *payload_blob_;
    }
  }  // namespace proto
}  // namespace shared_model

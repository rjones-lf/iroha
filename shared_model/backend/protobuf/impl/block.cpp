/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "backend/protobuf/block.hpp"

namespace shared_model {
  namespace proto {
    Block::Block(Block &&o) noexcept
        : AbstractNonCopyableProto(std::move(o.impl->getTransport())) {}

    Block &Block::operator=(Block &&o) noexcept {
      impl = std::move(o.impl);
      return *this;
    }

    interface::types::TransactionsCollectionType Block::transactions() const {
      return impl->transactions();
    }

    interface::types::HeightType Block::height() const {
      return impl->height();
    }

    const interface::types::HashType &Block::prevHash() const {
      return impl->prevHash();
    }

    const interface::types::BlobType &Block::blob() const {
      return impl->blob();
    }

    interface::types::SignatureRangeType Block::signatures() const {
      return impl->signatures();
    }

    bool Block::addSignature(const crypto::Signed &signed_blob,
                             const crypto::PublicKey &public_key) {
      return impl->addSignature(signed_blob, public_key);
    }

    interface::types::TimestampType Block::createdTime() const {
      return impl->createdTime();
    }

    interface::types::TransactionsNumberType Block::txsNumber() const {
      return impl->txsNumber();
    }

    const interface::types::BlobType &Block::payload() const {
      return impl->payload();
    }
  }  // namespace proto
}  // namespace shared_model

/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "backend/protobuf/block.hpp"

#include "backend/protobuf/common_objects/noncopyable_proto.hpp"
#include "backend/protobuf/common_objects/signature.hpp"
#include "backend/protobuf/transaction.hpp"
#include "backend/protobuf/util.hpp"
#include "interfaces/common_objects/types.hpp"

#include "block.pb.h"

namespace shared_model {
  namespace proto {

    struct Block::Impl {
      explicit Impl(TransportType &&ref) : proto_(std::move(ref)) {}
      explicit Impl(const TransportType &ref) : proto_(ref) {}
      Impl(Impl &&o) noexcept = delete;
      Impl &operator=(Impl &&o) noexcept = delete;

      TransportType proto_;
      iroha::protocol::Block::Payload &payload_{*proto_.mutable_payload()};

      std::vector<proto::Transaction> transactions_{[this] {
        return std::vector<proto::Transaction>(
            payload_.mutable_transactions()->begin(),
            payload_.mutable_transactions()->end());
      }()};

      interface::types::BlobType blob_{[this] { return makeBlob(proto_); }()};

      interface::types::HashType prev_hash_{[this] {
        return interface::types::HashType(proto_.payload().prev_block_hash());
      }()};

      SignatureSetType<proto::Signature> signatures_{[this] {
        auto signatures = proto_.signatures()
            | boost::adaptors::transformed([](const auto &x) {
                            return proto::Signature(x);
                          });
        return SignatureSetType<proto::Signature>(signatures.begin(),
                                                  signatures.end());
      }()};

      interface::types::BlobType payload_blob_{
          [this] { return makeBlob(payload_); }()};
    };

    Block::Block(Block &&o) noexcept = default;

    Block::Block(const TransportType &ref) {
      impl_ = std::make_unique<Block::Impl>(ref);
    }

    Block::Block(TransportType &&ref) {
      impl_ = std::make_unique<Block::Impl>(std::move(ref));
    }

    interface::types::TransactionsCollectionType Block::transactions() const {
      return impl_->transactions_;
    }

    interface::types::HeightType Block::height() const {
      return impl_->payload_.height();
    }

    const interface::types::HashType &Block::prevHash() const {
      return impl_->prev_hash_;
    }

    const interface::types::BlobType &Block::blob() const {
      return impl_->blob_;
    }

    interface::types::SignatureRangeType Block::signatures() const {
      return impl_->signatures_;
    }

    bool Block::addSignature(const crypto::Signed &signed_blob,
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

      auto sig = impl_->proto_.add_signatures();
      sig->set_signature(crypto::toBinaryString(signed_blob));
      sig->set_public_key(crypto::toBinaryString(public_key));

      impl_->signatures_ = [this] {
        auto signatures = impl_->proto_.signatures()
            | boost::adaptors::transformed([](const auto &x) {
                            return proto::Signature(x);
                          });
        return SignatureSetType<proto::Signature>(signatures.begin(),
                                                  signatures.end());
      }();
      return true;
    }

    interface::types::TimestampType Block::createdTime() const {
      return impl_->payload_.created_time();
    }

    interface::types::TransactionsNumberType Block::txsNumber() const {
      return impl_->payload_.tx_number();
    }

    const interface::types::BlobType &Block::payload() const {
      return impl_->payload_blob_;
    }

    const iroha::protocol::Block &Block::getTransport() const {
      return impl_->proto_;
    }

    Block::ModelType *Block::clone() const {
      return new Block(impl_->proto_);
    }

    Block::~Block() = default;
  }  // namespace proto
}  // namespace shared_model

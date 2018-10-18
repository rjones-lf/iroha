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

    struct Block::Impl : public NonCopyableProto<interface::Block,
                                                 iroha::protocol::Block,
                                                 Block::Impl> {
     public:
      using NonCopyableProto::NonCopyableProto;

      Impl(Impl &&o) noexcept : NonCopyableProto(std::move(o.proto_)) {}
      Impl &operator=(Impl &&o) noexcept {
        proto_ = std::move(o.proto_);
        payload_ = *proto_.mutable_payload();
        return *this;
      }

      interface::types::TransactionsCollectionType transactions()
          const override {
        return transactions_;
      }

      interface::types::HeightType height() const override {
        return payload_.height();
      }

      const interface::types::HashType &prevHash() const override {
        return prev_hash_;
      }

      const interface::types::BlobType &blob() const override {
        return blob_;
      }

      interface::types::SignatureRangeType signatures() const override {
        return signatures_;
      }

      bool addSignature(const crypto::Signed &signed_blob,
                        const crypto::PublicKey &public_key) override {
        // if already has such signature
        if (std::find_if(signatures_.begin(),
                         signatures_.end(),
                         [&public_key](const auto &signature) {
                           return signature.publicKey() == public_key;
                         })
            != signatures_.end()) {
          return false;
        }

        auto sig = proto_.add_signatures();
        sig->set_signature(crypto::toBinaryString(signed_blob));
        sig->set_public_key(crypto::toBinaryString(public_key));

        signatures_ = [this] {
          auto signatures = proto_.signatures()
              | boost::adaptors::transformed([](const auto &x) {
                              return proto::Signature(x);
                            });
          return SignatureSetType<proto::Signature>(signatures.begin(),
                                                    signatures.end());
        }();

        return true;
      }

      interface::types::TimestampType createdTime() const override {
        return payload_.created_time();
      }

      interface::types::TransactionsNumberType txsNumber() const override {
        return payload_.tx_number();
      }

      const interface::types::BlobType &payload() const override {
        return payload_blob_;
      }

     private:
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

    Block::Block(Block &&o) noexcept
        : Block(std::move(o.impl->getTransport())) {}

    Block &Block::operator=(Block &&o) noexcept {
      impl = std::move(o.impl);
      return *this;
    }

    Block::Block(iroha::protocol::Block ref) {
      impl = std::make_unique<Block::Impl>(std::move(ref));
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

    const iroha::protocol::Block &Block::getTransport() const {
      return impl->getTransport();
    }

    Block::ModelType *Block::clone() const {
      return new Block(std::move(impl->getTransport()));
    }

    Block::~Block() = default;
  }  // namespace proto
}  // namespace shared_model

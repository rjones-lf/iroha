/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_PROTO_SIGNATURE_HPP
#define IROHA_PROTO_SIGNATURE_HPP

#include "interfaces/common_objects/signature.hpp"

#include "backend/protobuf/common_objects/trivial_proto.hpp"
#include "cryptography/public_key.hpp"
#include "cryptography/signed.hpp"
#include "primitive.pb.h"

namespace shared_model {
  namespace proto {
    class Signature final : public CopyableProto<interface::Signature,
                                                 iroha::protocol::Signature,
                                                 Signature> {
     public:
      template <typename SignatureType>
      explicit Signature(SignatureType &&signature)
          : CopyableProto(std::forward<SignatureType>(signature)) {}

      Signature(const Signature &o) : Signature(o.proto_) {}

      Signature(Signature &&o) noexcept : Signature(std::move(o.proto_)) {}

      const PublicKeyType &publicKey() const override {
        return *public_key_;
      }

      const SignedType &signedData() const override {
        return *signed_;
      }

     private:
      // lazy
      template <typename T>
      using Lazy = detail::LazyInitializer<T>;

      const Lazy<PublicKeyType> public_key_{
          [this] { return PublicKeyType(proto_->public_key()); }};

      const Lazy<SignedType> signed_{
          [this] { return SignedType(proto_->signature()); }};
    };
  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_PROTO_SIGNATURE_HPP

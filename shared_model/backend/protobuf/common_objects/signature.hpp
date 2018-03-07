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

#ifndef IROHA_PROTO_SIGNATURE_HPP
#define IROHA_PROTO_SIGNATURE_HPP

#include "backend/protobuf/common_objects/trivial_proto.hpp"
#include "interfaces/common_objects/signature.hpp"
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
          [this] { return PublicKeyType(proto_->pubkey()); }};

      const Lazy<SignedType> signed_{
          [this] { return SignedType(proto_->signature()); }};
    };
  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_PROTO_SIGNATURE_HPP

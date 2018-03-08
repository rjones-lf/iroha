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

#ifndef IROHA_UNSIGNED_PROTO_HPP
#define IROHA_UNSIGNED_PROTO_HPP

#include "backend/protobuf/common_objects/signature.hpp"
#include "cryptography/crypto_provider/crypto_signer.hpp"
#include "cryptography/keypair.hpp"
#include "interfaces/common_objects/types.hpp"
#include "utils/polymorphic_wrapper.hpp"

namespace shared_model {
  namespace proto {
    /**
     * Class for holding built but still unsigned objects
     * @tparam T - type of object received from builder
     */
    template <typename T>
    class UnsignedWrapper {
     public:
      using ModelType = T;

      /**
       * Constructs new unsigned object instance
       * @param o - object received from builder
       */
      explicit UnsignedWrapper(const T &o) : unsigned_(o) {}

      explicit UnsignedWrapper(T &&o) : unsigned_(std::move(o)) {}

      /**
       * Add signature and retrieve signed result
       * @param signature - signature to add
       * @return signed object
       */
      T signAndAddSignature(const crypto::Keypair &keypair) {
        auto signedBlob = shared_model::crypto::CryptoSigner<>::sign(
            shared_model::crypto::Blob(unsigned_.payload()), keypair);
        iroha::protocol::Signature protosig;
        protosig.set_pubkey(crypto::toBinaryString(keypair.publicKey()));
        protosig.set_signature(crypto::toBinaryString(signedBlob));
        auto *s1 = new Signature(protosig);
        unsigned_.addSignature(detail::PolymorphicWrapper<Signature>(
            s1));  // TODO: 05.12.2017 luckychess think about false case
        return unsigned_;
      }

      interface::types::HashType hash() {
        return unsigned_.hash();
      }

     private:
      T unsigned_;
    };
  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_UNSIGNED_PROTO_HPP

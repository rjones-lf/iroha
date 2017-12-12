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

#ifndef IROHA_SHARED_MODEL_KEYPAIR_HPP
#define IROHA_SHARED_MODEL_KEYPAIR_HPP

#include "cryptography/private_key.hpp"
#include "cryptography/public_key.hpp"
#include "interfaces/base/primitive.hpp"
#include "utils/string_builder.hpp"

#include "common/types.hpp"

namespace shared_model {
  namespace crypto {
    /**
     * Class for holding a keypair: public key and private key
     */
    class Keypair : public interface::Primitive<Keypair, iroha::keypair_t> {
     public:
      /// Type of public key
      using PublicKeyType = PublicKey;

      /// Type of private key
      using PrivateKeyType = PrivateKey;

      explicit Keypair(PublicKeyType publicKey, PrivateKeyType privateKey)
          : publicKey_(publicKey), privateKey_(privateKey) {}

      /**
       * @return public key
       */
      const PublicKeyType &publicKey() const { return publicKey_; };

      /**
       * @return private key
       */
      const PrivateKeyType &privateKey() const { return privateKey_; };

      bool operator==(const Keypair &keypair) const override {
        return publicKey() == keypair.publicKey()
            and privateKey() == keypair.privateKey();
      }

      std::string toString() const override {
        return detail::PrettyStringBuilder()
            .init("Keypair")
            .append("publicKey", publicKey().toString())
            .append("privateKey", privateKey().toString())
            .finalize();
      }

      interface::Primitive<Keypair, iroha::keypair_t>::OldModelType *
      makeOldModel() const override {
        return new iroha::keypair_t{
            publicKey().makeOldModel<PublicKey::OldPublicKeyType>(),
            privateKey().makeOldModel<PrivateKey::OldPrivateKeyType>()};
      }

      Keypair *copy() const override {
        return new Keypair(publicKey(), privateKey());
      };

     private:
      PublicKey publicKey_;
      PrivateKey privateKey_;
    };
  }  // namespace crypto
}  // namespace shared_model

#endif  // IROHA_SHARED_MODEL_KEYPAIR_HPP

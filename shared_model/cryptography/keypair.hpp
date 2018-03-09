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

#include "common/types.hpp"  // for keypair_t

namespace shared_model {
  namespace crypto {

    using KeypairOldModelType = iroha::keypair_t;

    /**
     * Class for holding a keypair: public key and private key
     */
#ifndef DISABLE_BACKWARD
    class Keypair : public interface::Primitive<Keypair, KeypairOldModelType> {
#else
    class Keypair : public interface::ModelPrimitive<Keypair> {
#endif
     public:
      /// Type of public key
      using PublicKeyType = PublicKey;

      /// Type of private key
      using PrivateKeyType = PrivateKey;

      explicit Keypair(const PublicKeyType &public_key,
                       const PrivateKeyType &private_key);

      /**
       * @return public key
       */
      const PublicKeyType &publicKey() const;

      /**
       * @return private key
       */
      const PrivateKeyType &privateKey() const;

      bool operator==(const Keypair &keypair) const override;

      std::string toString() const override;

#ifndef DISABLE_BACKWARD
      KeypairOldModelType *makeOldModel() const override;

#endif

      Keypair *copy() const override;

     private:
      PublicKey public_key_;
      PrivateKey private_key_;
    };
  }  // namespace crypto
}  // namespace shared_model

#endif  // IROHA_SHARED_MODEL_KEYPAIR_HPP

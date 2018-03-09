/**
 * Copyright Soramitsu Co., Ltd. 2018 All Rights Reserved.
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

#include "cryptography/keypair.hpp"

#include "utils/string_builder.hpp"

namespace shared_model {
  namespace crypto {

    const Keypair::PublicKeyType &Keypair::publicKey() const {
      return public_key_;
    }

    const Keypair::PrivateKeyType &Keypair::privateKey() const {
      return private_key_;
    }

    bool Keypair::operator==(const Keypair &keypair) const {
      return publicKey() == keypair.publicKey()
          and privateKey() == keypair.privateKey();
    }

    std::string Keypair::toString() const {
      return detail::PrettyStringBuilder()
          .init("Keypair")
          .append("publicKey", publicKey().toString())
          .append("privateKey", privateKey().toString())
          .finalize();
    }

    Keypair *Keypair::copy() const {
      return new Keypair(publicKey(), privateKey());
    }

#ifndef DISABLE_BACKWARD
    KeypairOldModelType *Keypair::makeOldModel() const {
      return new iroha::keypair_t{
          publicKey().makeOldModel<PublicKey::OldPublicKeyType>(),
          privateKey().makeOldModel<PrivateKey::OldPrivateKeyType>()};
    }
#endif

    Keypair::Keypair(const Keypair::PublicKeyType &public_key,
                     const Keypair::PrivateKeyType &private_key)
        : public_key_(public_key), private_key_(private_key) {}

  }  // namespace crypto
}  // namespace shared_model

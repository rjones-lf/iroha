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

#include "bindings/model_crypto.hpp"
#include "cryptography/ed25519_sha3_impl/crypto_provider.hpp"

namespace shared_model {
  namespace bindings {
    crypto::Keypair ModelCrypto::generateKeypair() {
      return crypto::CryptoProviderEd25519Sha3::generateKeypair();
    }

    crypto::Keypair ModelCrypto::generateKeypair(const std::string &seed) {
      return crypto::CryptoProviderEd25519Sha3::generateKeypair(
          crypto::Seed(seed));
    }

    crypto::Keypair ModelCrypto::convertFromExisting(
        const std::string &publicKey, const std::string &privateKey) {
      crypto::Keypair keypair((crypto::Keypair::PublicKeyType(
                                  crypto::Blob::fromHexString(publicKey))),
                              crypto::Keypair::PrivateKeyType(
                                  crypto::Blob::fromHexString(privateKey)));

      auto randStr = iroha::randomString(32);
      if (not crypto::CryptoProviderEd25519Sha3::verify(
              crypto::CryptoProviderEd25519Sha3::sign(crypto::Blob(randStr),
                                                      keypair),
              crypto::Blob(randStr),
              keypair.publicKey())) {
        throw std::invalid_argument("Provided keypair is not correct");
      }

      return keypair;
    }
  }  // namespace bindings
}  // namespace shared_model

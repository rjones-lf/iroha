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

#ifndef IROHA_SHARED_MODEL_MODEL_CRYPTO_HPP
#define IROHA_SHARED_MODEL_MODEL_CRYPTO_HPP

#include "cryptography/blob.hpp"
#include "cryptography/keypair.hpp"
#include "cryptography/signed.hpp"

namespace shared_model {
  namespace bindings {
    /**
     * Class for crypto operations for SWIG
     */
    class ModelCrypto {
     public:
      /**
       * Generates new keypair (ed25519)
       * @return generated keypair
       */
      crypto::Keypair generateKeypair();

      /**
       * Creates keypair (ed25519) from provided private key
       * @param private_key - ed25519 hex-encoded private key
       * @return created keypair
       */
      crypto::Keypair fromPrivateKey(const std::string &private_key);

      /**
       * Retrieves Keypair object (ed25519) from existing keypair.
       * @param public_key - ed25519 hex-encoded public key
       * @param private_key - ed25519 hex-encoded private key
       * @return keypair from provided keys
       */
      crypto::Keypair convertFromExisting(const std::string &public_key,
                                          const std::string &private_key);
    };
  }  // namespace bindings
}  // namespace shared_model

#endif  // IROHA_SHARED_MODEL_MODEL_CRYPTO_HPP

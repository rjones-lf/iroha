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

#ifndef IROHA_CRYPTOPROVIDER_HPP
#define IROHA_CRYPTOPROVIDER_HPP

#include "cryptography/keypair.hpp"
#include "cryptography/seed.hpp"
#include "cryptography/signed.hpp"

namespace shared_model {
  namespace crypto {
    /**
     * Wrapper class for signing-related stuff.
     */
    class CryptoProviderEd25519Sha3 {
     public:
      /**
       * Signs the message.
       * @param blob - blob to sign
       * @param keypair - keypair
       * @return Signed object with signed data
       */
      static Signed sign(const Blob &blob, const Keypair &keypair);

      /**
       * Verifies signature.
       * @param signedData - data to verify
       * @param orig - original message
       * @param publicKey - public key
       * @return true if verify was OK or false otherwise
       */
      static bool verify(const Signed &signedData,
                         const Blob &orig,
                         const PublicKey &publicKey);
      /**
       * Generates new seed
       * @return Seed generated
       */
      static Seed generateSeed();

      /**
       * Generates new seed from a provided passphrase
       * @param passphrase - passphrase to generate seed from
       * @return Seed generated
       */
      static Seed generateSeed(const std::string &passphrase);

      /**
       * Generates new keypair with a default seed
       * @return Keypair generated
       */
      static Keypair generateKeypair();

      /**
       * Generates new keypair from a provided seed
       * @param seed - provided seed
       * @return generated keypair
       */
      static Keypair generateKeypair(const Seed &seed);

      static const size_t kHashLength;
      static const size_t kPublicKeyLength;
      static const size_t kPublicKeyHexLength;
      static const size_t kPrivateKeyLength;
      static const size_t kSignatureLength;
      static const size_t kSeedLength;
    };
  }  // namespace crypto
}  // namespace shared_model

#endif  // IROHA_CRYPTOPROVIDER_HPP

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

#ifndef IROHA_CRYPTO_VERIFIER_HPP
#define IROHA_CRYPTO_VERIFIER_HPP

#include "cryptography/crypto_provider/crypto_defaults.hpp"

namespace shared_model {
  namespace crypto {

    class Signed;
    class Blob;
    class PublicKey;

    /**
     * CryptoVerifier - adapter for generalization verification of cryptographic
     * signatures
     * @tparam Algorithm - cryptographic algorithm for verification
     */
    template <typename Algorithm = DefaultCryptoAlgorithmType>
    class CryptoVerifier {
     public:
      /**
       * Verify signature attached to source data
       * @param signedData - cryptographic signature
       * @param source - data that was signed
       * @param pubKey - public key of signatory
       * @return true if signature correct
       */
      static bool verify(const Signed &signedData,
                         const Blob &source,
                         const PublicKey &pubKey) {
        return Algorithm::verify(signedData, source, pubKey);
      }

      /// close constructor for forbidding instantiation
      CryptoVerifier() = delete;
    };
  }  // namespace crypto
}  // namespace shared_model

#endif  // IROHA_CRYPTO_VERIFIER_HPP

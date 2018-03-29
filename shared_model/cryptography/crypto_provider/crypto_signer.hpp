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

#ifndef IROHA_CRYPTO_SIGNER_HPP
#define IROHA_CRYPTO_SIGNER_HPP

#include "cryptography/blob.hpp"
#include "cryptography/crypto_provider/crypto_defaults.hpp"
#include "cryptography/keypair.hpp"
#include "cryptography/signed.hpp"

namespace shared_model {
  namespace crypto {
    /**
     * CryptoSigner - wrapper for generalization signing for different
     * cryptographic algorithms
     * @tparam Algorithm - cryptographic algorithm for singing
     */
    template <typename Algorithm = DefaultCryptoAlgorithmType>
    class CryptoSigner {
     public:
      /**
       * Generate signature for target data
       * @param blob - data for signing
       * @param keypair - (public, private) keys for signing
       * @return signature's blob
       */
      static Signed sign(const Blob &blob, const Keypair &keypair) {
        return Algorithm::sign(blob, keypair);
      }

      /// close constructor for forbidding instantiation
      CryptoSigner() = delete;
    };
  }  // namespace crypto
}  // namespace shared_model
#endif  // IROHA_CRYPTO_SIGNER_HPP

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

#ifndef IROHA_CRYPTO_VERIFIER_IMPL_HPP
#define IROHA_CRYPTO_VERIFIER_IMPL_HPP

#include "cryptography/crypto_provider/crypto_defaults.hpp"
#include "cryptography/crypto_provider/crypto_verifier.hpp"
#include "interfaces/common_objects/signature.hpp"
#include "interfaces/iroha_internal/block.hpp"
#include "interfaces/queries/query.hpp"
#include "interfaces/transaction.hpp"
#include "utils/polymorphic_wrapper.hpp"

namespace shared_model {
  namespace crypto {
    /**
     * CryptoVerifierImpl - implementation of verification of cryptographic
     * signatures with specific cryptographic algorithm
     * @tparam Algorithm - cryptographic algorithm for verification
     */
    template <typename Algorithm = DefaultCryptoAlgorithmType>
    class CryptoVerifierImpl : public CryptoVerifier {
     public:
      virtual ~CryptoVerifierImpl() = default;

      bool verify(const shared_model::interface::Block &block) const override;

      bool verify(const shared_model::interface::Query &query) const override;

      bool verify(
          const shared_model::interface::Transaction &tx) const override;

     private:
      template <typename T>
      bool internal_verify(const T &signable) const;
    };

    template <typename Algorithm>
    bool CryptoVerifierImpl<Algorithm>::verify(
        const shared_model::interface::Block &block) const {
      return internal_verify(block);
    }

    template <typename Algorithm>
    bool CryptoVerifierImpl<Algorithm>::verify(
        const shared_model::interface::Query &query) const {
      return internal_verify(query);
    }

    template <typename Algorithm>
    bool CryptoVerifierImpl<Algorithm>::verify(
        const shared_model::interface::Transaction &tx) const {
      return internal_verify(tx);
    }

    template <typename Algorithm>
    template <typename T>
    bool CryptoVerifierImpl<Algorithm>::internal_verify(
        const T &signable) const {
      return signable.signatures().size() > 0
          and std::all_of(
                  signable.signatures().begin(),
                  signable.signatures().end(),
                  [this,
                   &signable](const shared_model::detail::PolymorphicWrapper<
                              shared_model::interface::Signature> &signature) {
                    return Algorithm::verify(signature->signedData(),
                                             signable.payload(),
                                             signature->publicKey());
                  });
    }

  }  // namespace crypto
}  // namespace shared_model

#endif  // IROHA_CRYPTO_VERIFIER_IMPL_HPP

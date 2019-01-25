/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_ABSTRACT_CRYPTO_MODEL_SIGNER_HPP
#define IROHA_ABSTRACT_CRYPTO_MODEL_SIGNER_HPP

namespace shared_model {
  namespace crypto {

    // signer which can be overloaded for dynamic polymorphism
    template <typename Model>
    class AbstractCryptoModelSigner {
     public:
      virtual void sign(Model &m) const = 0;

      virtual ~AbstractCryptoModelSigner() = default;
    };

  }  // namespace crypto
}  // namespace shared_model

#endif  // IROHA_ABSTRACT_CRYPTO_MODEL_SIGNER_HPP

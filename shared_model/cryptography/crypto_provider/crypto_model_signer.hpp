/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_CRYPTO_MODEL_SIGNER_HPP_
#define IROHA_CRYPTO_MODEL_SIGNER_HPP_

#include "cryptography/crypto_provider/crypto_signer.hpp"

namespace shared_model {

  namespace interface {
    class Block;
    class Query;
    class Transaction;
  }

  namespace crypto {
    template <typename Algorithm = CryptoSigner<>>
    class CryptoModelSigner {
     public:
      explicit CryptoModelSigner(const shared_model::crypto::Keypair &keypair);

      virtual ~CryptoModelSigner() = default;

      template <typename T>
      void sign(T &signable) const noexcept {
        auto signedBlob = Algorithm::sign(signable.payload(), keypair_);
        signable.addSignature(signedBlob, keypair_.publicKey());
      }

     private:
      shared_model::crypto::Keypair keypair_;
    };

    template <typename Algorithm>
    CryptoModelSigner<Algorithm>::CryptoModelSigner(
        const shared_model::crypto::Keypair &keypair)
        : keypair_(keypair) {}

  }  // namespace crypto
}  // namespace shared_model

#endif  //  IROHA_CRYPTO_MODEL_SIGNER_HPP_

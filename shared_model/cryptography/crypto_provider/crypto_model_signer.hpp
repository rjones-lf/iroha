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

#ifndef IROHA_CRYPTO_MODEL_SIGNER_HPP_
#define IROHA_CRYPTO_MODEL_SIGNER_HPP_

#include "cryptography/crypto_provider/crypto_defaults.hpp"
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

      virtual void sign(shared_model::interface::Block &block) const;

      virtual void sign(shared_model::interface::Query &query) const;

      virtual void sign(
          shared_model::interface::Transaction &transaction) const;

     private:
      template <typename T>
      void internal_sign(T &signable) const noexcept {
        auto signedBlob = Algorithm::sign(signable.payload(), keypair_);
        signable.addSignature(signedBlob, keypair_.publicKey());
      }

      shared_model::crypto::Keypair keypair_;
    };

    template <typename Algorithm>
    CryptoModelSigner<Algorithm>::CryptoModelSigner(
        const shared_model::crypto::Keypair &keypair)
        : keypair_(keypair) {}

    template <typename Algorithm>
    void CryptoModelSigner<Algorithm>::sign(
        shared_model::interface::Block &block) const {
      internal_sign(block);
    }

    template <typename Algorithm>
    void CryptoModelSigner<Algorithm>::sign(
        shared_model::interface::Query &query) const {
      internal_sign(query);
    }

    template <typename Algorithm>
    void CryptoModelSigner<Algorithm>::sign(
        shared_model::interface::Transaction &transaction) const {
      internal_sign(transaction);
    }

  }  // namespace crypto
}  // namespace shared_model

#endif  //  IROHA_CRYPTO_MODEL_SIGNER_HPP_

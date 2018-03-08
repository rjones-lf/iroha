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

#include "backend/protobuf/common_objects/signature.hpp"
#include "cryptography/crypto_provider/crypto_signer.hpp"
#include "cryptography/keypair.hpp"
#include "interfaces/base/signable.hpp"
#include "interfaces/common_objects/signature.hpp"

namespace shared_model {
  namespace crypto {

    /**
     * Wrapper for generalization signing for different model types.
     * @tparam Algorithm - wrapper for generalization signing for different
     * cryptographic algorithms
     */
    template <typename Algorithm = DefaultCryptoAlgorithmType>
    class CryptoModelSigner {
     public:
      CryptoModelSigner(crypto::Keypair kp);

      /**
       * Generate and add signature for target data
       * @param signable - data for signing
       * @return signature's blob
       */
#ifndef DISABLE_BACKWARD
      template <typename Model, typename OldModel>
      void sign(interface::Signable<Model, OldModel> &signable) const noexcept;
#else
      template <typename Model>
      void sign(interface::Signable<Model> &signable) const noexcept;
#endif

     private:
      Keypair keypair_;
    };

    template <typename Algorithm>
    CryptoModelSigner<Algorithm>::CryptoModelSigner(crypto::Keypair kp)
        : keypair_(std::move(kp)) {}

    template <typename Algorithm>
#ifndef DISABLE_BACKWARD
    template <typename Model, typename OldModel>
    void CryptoModelSigner<Algorithm>::sign(
        interface::Signable<Model, OldModel> &signable) const noexcept {
#else
    template <typename Model>
    void CryptoModelSigner<Algorithm>::sign(
        interface::Signable<Model> &signable) const noexcept {
#endif
      auto signedBlob = shared_model::crypto::CryptoSigner<Algorithm>::sign(
          shared_model::crypto::Blob(signable.payload()), keypair_);
      iroha::protocol::Signature protosig;
      protosig.set_pubkey(
          shared_model::crypto::toBinaryString(keypair_.publicKey()));
      protosig.set_signature(shared_model::crypto::toBinaryString(signedBlob));
      auto *signature = new shared_model::proto::Signature(protosig);
      signable.addSignature(shared_model::detail::PolymorphicWrapper<
                            shared_model::proto::Signature>(signature));
    }

  }  // namespace crypto
}  // namespace shared_model

#endif  //  IROHA_CRYPTO_MODEL_SIGNER_HPP_

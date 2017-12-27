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

#ifndef IROHA_CRYPTO_MODEL_SIGNER_HPP_
#define IROHA_CRYPTO_MODEL_SIGNER_HPP_

#include "cryptography/keypair.hpp"
#include "interfaces/base/signable.hpp"
#include "interfaces/common_objects/signature.hpp"

namespace shared_model {
  namespace crypto {

    template <typename SignerT>
    class CryptoModelSigner {
     public:
      CryptoModelSigner(crypto::Keypair kp);

      void sign(interface::Signable &s) noexcept;

     private:
      crypto::Keypair keypair_;
    };

    /// implementation
    template <typename SignerT>
    void CryptoModelSigner<SignerT>::sign(interface::Signable &s) noexcept {
      auto sigblob = SignerT::sign(s.hash(), this->keypair_);
    }

    template <typename SignerT>
    CryptoModelSigner<SignerT>::CryptoModelSigner(crypto::Keypair kp)
        : keypair_(std::move(kp)) {}
  }
}

#endif  //  IROHA_CRYPTO_MODEL_SIGNER_HPP_

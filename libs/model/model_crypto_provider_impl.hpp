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

#ifndef IROHA_MODEL_CRYPTO_PROVIDER_IMPL_HPP
#define IROHA_MODEL_CRYPTO_PROVIDER_IMPL_HPP

#include <model/model_crypto_provider.hpp>
#include <model/model_hash_provider.hpp>

namespace iroha {
  namespace model {

    class ModelCryptoProviderImpl : public ModelCryptoProvider {
     public:
      ModelCryptoProviderImpl(ed25519::privkey_t privkey,
                            ed25519::pubkey_t pubkey);

      bool verify(const Transaction &tx) override;

      Transaction &sign(Transaction &tx) override;

     private:
      ed25519::privkey_t privkey_;
      ed25519::pubkey_t pubkey_;
    };
  }
}

#endif  // IROHA_MODEL_CRYPTO_PROVIDER_IMPL_HPP

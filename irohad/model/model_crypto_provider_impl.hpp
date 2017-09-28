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

#include <model/converters/pb_common.hpp>
#include <model/model_crypto_provider.hpp>

namespace iroha {
  namespace model {

    class ModelCryptoProviderImpl : public ModelCryptoProvider {
     public:
      bool verify(const Transaction& tx) const override;
      bool verify(std::shared_ptr<const Query> tx) const override;
      bool verify(const Block& block) const override;

      /**
       * Sign block with given keypair. Adds signature to the array of sigs.
       */
      void sign(model::Block& b, keypair_t const& kp) const override;

      /**
       * Sign transaction with given keypair.  Adds signature to the array of
       * signatures.
       */
      void sign(model::Transaction& b, keypair_t const& kp) const override;

      /**
       * Sign query. Replaces current signature with new one.
       */
      void sign(model::Query& b, keypair_t const& kp) const override;
    };
  }  // namespace model
}  // namespace iroha

#endif  // IROHA_MODEL_CRYPTO_PROVIDER_IMPL_HPP

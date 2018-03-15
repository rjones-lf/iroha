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

#ifndef IROHA_CRYPTO_PROVIDER_IMPL_HPP
#define IROHA_CRYPTO_PROVIDER_IMPL_HPP

#include <memory>

#include "crypto_provider/crypto_provider.hpp"
#include "cryptography/crypto_provider/crypto_defaults.hpp"
#include "cryptography/crypto_provider/crypto_model_signer.hpp"
#include "cryptography/keypair.hpp"

#include <algorithm>

#include "interfaces/iroha_internal/block.hpp"
#include "interfaces/queries/query.hpp"
#include "interfaces/transaction.hpp"

#include "cryptography/crypto_provider/crypto_verifier.hpp"
#include "cryptography/ed25519_sha3_impl/internal/ed25519_impl.hpp"
#include "model/block.hpp"
#include "model/query.hpp"
#include "model/sha3_hash.hpp"
#include "model/transaction.hpp"

namespace iroha {

  template <typename Algorithm =
                shared_model::crypto::DefaultCryptoAlgorithmType>
  class CryptoProviderImpl : public CryptoProvider {
   public:
    explicit CryptoProviderImpl(const shared_model::crypto::Keypair &keypair);

    void sign(shared_model::interface::Block &block) const override;

    void sign(shared_model::interface::Query &query) const override;

    void sign(shared_model::interface::Transaction &transaction) const override;

   private:
    shared_model::crypto::Keypair keypair_;
    shared_model::crypto::CryptoModelSigner<Algorithm> signer_;
  };

  template <typename Algorithm>
  CryptoProviderImpl<Algorithm>::CryptoProviderImpl(
      const shared_model::crypto::Keypair &keypair)
      : keypair_(keypair), signer_(keypair_) {}

  template <typename Algorithm>
  void CryptoProviderImpl<Algorithm>::sign(
      shared_model::interface::Block &block) const {
    signer_.sign(block);
  }

  template <typename Algorithm>
  void CryptoProviderImpl<Algorithm>::sign(
      shared_model::interface::Query &query) const {
    signer_.sign(query);
  }

  template <typename Algorithm>
  void CryptoProviderImpl<Algorithm>::sign(
      shared_model::interface::Transaction &transaction) const {
    signer_.sign(transaction);
  }

}  // namespace iroha

#endif  // IROHA_CRYPTO_PROVIDER_IMPL_HPP

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


#include <model/model_crypto_provider_impl.hpp>
#include <model/model_hash_provider_impl.hpp>

namespace iroha {
  namespace model {

  ModelCryptoProviderImpl::ModelCryptoProviderImpl(ed25519::privkey_t privkey,
  ed25519::pubkey_t pubkey)
  : privkey_(privkey), pubkey_(pubkey) {}


  bool ModelCryptoProviderImpl::verify(const Transaction &tx) {
    HashProviderImpl hash_provider;
    auto tx_hash = hash_provider.get_hash(tx);
    for (auto sign: tx.signatures) {
      auto verified = iroha::verify(tx_hash.data(), tx_hash.size(), sign.pubkey, sign.signature);
      if (!verified)
        return false;
    }
    return true;
  }

  Transaction &ModelCryptoProviderImpl::sign(Transaction &tx) {
    model::HashProviderImpl hash_provider;
    auto tx_hash = hash_provider.get_hash(tx);

    auto sign = iroha::sign(tx_hash.data(), tx_hash.size(), pubkey_, privkey_);

    Signature signature{};
    signature.signature = sign;
    signature.pubkey = pubkey_;

    tx.signatures.push_back(signature);

    return tx;
  }

  }
}
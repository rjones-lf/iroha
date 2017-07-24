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

    bool ModelCryptoProviderImpl::verify(const Transaction &tx) const {
      HashProviderImpl hash_provider;  // TODO: get rid off variable
                                       // initialization on every verify call
      auto tx_hash = hash_provider.get_hash(tx);

      if (tx.signatures.size() == 0) return false;

      for (auto sign : tx.signatures) {
        auto verified = iroha::verify(tx_hash.data(), tx_hash.size(),
                                      sign.pubkey, sign.signature);
        if (!verified) return false;
      }
      return true;
    }

    bool ModelCryptoProviderImpl::verify(const Query &query) const {
      HashProviderImpl hashProvider;  // TODO: get rid off variable
                                      // initialization on every verify call
      auto query_hash = hashProvider.get_hash(query);
      auto sign = query.signature;
      return iroha::verify(query_hash.data(), query_hash.size(), sign.pubkey,
                           sign.signature);
    }

    bool ModelCryptoProviderImpl::verify(const Block &block) const {
      HashProviderImpl hashProvider;  // TODO: get rid off variable
                                      // initialization on every verify call
      auto block_hash = hashProvider.get_hash(block);

      if (block.sigs.size() == 0) {
        return false;
      }

      for (auto sign : block.sigs) {
        auto verified = iroha::verify(block_hash.data(), block_hash.size(),
                                      sign.pubkey, sign.signature);
        if (!verified) return false;
      }
      return true;
    }

    void ModelCryptoProviderImpl::sign(const Block &block) const {
      HashProviderImpl hashProvider;  // TODO: get rid off variable
                                      // initialization on every verify call
      auto block_hash = hashProvider.get_hash(block);
      auto sig =
          iroha::sign(block_hash.data(), block_hash.size(), pubkey_, privkey_);
      Signature signature;
      signature.pubkey = pubkey_;
      signature.signature = sig;
      block.sigs.push_back(signature);
    }
  }
}
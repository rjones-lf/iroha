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

#include <crypto/crypto.hpp>
#include <crypto/hash.hpp>
#include <model/converters/pb_block_factory.hpp>
#include <model/converters/pb_query_factory.hpp>
#include <model/converters/pb_transaction_factory.hpp>
#include <model/model_crypto_provider_impl.hpp>

namespace iroha {
  namespace model {

    bool ModelCryptoProviderImpl::verify(const Transaction &tx) const {
      if (tx.signatures.empty()) {
        return false;
      }

      const auto hash_ = hash(tx).to_string();

      for (const auto &sign : tx.signatures) {
        if (not iroha::verify(hash_, sign.pubkey, sign.signature)) {
          return false;
        }
      }
      return true;
    }

    bool ModelCryptoProviderImpl::verify(
        std::shared_ptr<const Query> query) const {
      const auto hash_ = hash(*query).to_string();
      const auto sig = query->signature;

      return iroha::verify(hash_, sig.pubkey, sig.signature);
    }

    bool ModelCryptoProviderImpl::verify(const Block &block) const {
      const auto hash_ = hash(block).to_string();

      for (const auto &sig : block.sigs) {
        if (not iroha::verify(hash_, sig.pubkey, sig.signature)) {
          return false;
        }
      }
      return true;
    }

    void ModelCryptoProviderImpl::sign(model::Block &b, keypair_t const &kp) const {
      const auto hash_ = hash(b).to_string();
      const auto signature = iroha::sign(hash_, kp.pubkey, kp.privkey);
      b.sigs.emplace_back(model::Signature{signature, kp.pubkey});
    }

    void ModelCryptoProviderImpl::sign(model::Transaction &b,
                                       keypair_t const &kp) const {
      const auto hash_ = hash(b).to_string();
      const auto signature = iroha::sign(hash_, kp.pubkey, kp.privkey);
      b.signatures.emplace_back(model::Signature{signature, kp.pubkey});
    }

    void ModelCryptoProviderImpl::sign(model::Query &b, keypair_t const &kp) const {
      const auto hash_ = hash(b).to_string();
      const auto signature = iroha::sign(hash_, kp.pubkey, kp.privkey);
      b.signature = model::Signature{signature, kp.pubkey};
    }

  }  // namespace model
}  // namespace iroha

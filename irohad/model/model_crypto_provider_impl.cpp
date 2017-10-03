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

#include "model_crypto_provider_impl.hpp"
#include "crypto/crypto.hpp"
#include "crypto/hash.hpp"

namespace iroha {
  namespace model {
    ModelCryptoProviderImpl::ModelCryptoProviderImpl(const keypair_t &keypair)
        : keypair_(keypair) {}

    bool ModelCryptoProviderImpl::verify(const Transaction &tx) const {
      return std::all_of(tx.signatures.begin(),
                         tx.signatures.end(),
                         [tx](const Signature &sig) {
                           return iroha::verify(iroha::hash(tx).to_string(),
                                                sig.pubkey,
                                                sig.signature);
                         });
    }

    bool ModelCryptoProviderImpl::verify(
        std::shared_ptr<const Query> query) const {
      return iroha::verify(
          iroha::hash(*query).to_string(), keypair_.pubkey, keypair_.privkey);
    }

    bool ModelCryptoProviderImpl::verify(const Block &block) const {
      return std::all_of(
          block.sigs.begin(), block.sigs.end(), [block](const Signature &sig) {
            return iroha::verify(
                iroha::hash(block).to_string(), sig.pubkey, sig.signature);
          });
    }

    Block ModelCryptoProviderImpl::sign(const Block &block) const {
      auto signature = iroha::sign(
          iroha::hash(block).to_string(), keypair_.pubkey, keypair_.privkey);
      auto signed_block = block;
      signed_block.sigs.push_back(Signature{signature, keypair_.pubkey});
      return signed_block;
    }

    Transaction ModelCryptoProviderImpl::sign(
        const Transaction &transaction) const {
      auto signature = iroha::sign(iroha::hash(transaction).to_string(),
                                   keypair_.pubkey,
                                   keypair_.privkey);
      auto signed_transaction = transaction;
      signed_transaction.signatures.push_back(
          Signature{signature, keypair_.pubkey});
      return signed_transaction;
    }
  }
}

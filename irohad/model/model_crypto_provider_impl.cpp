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

#include "model/model_crypto_provider_impl.hpp"
#include <algorithm>
#include "cryptography/ed25519_sha3_impl/internal/ed25519_impl.hpp"
#include "model/queries/get_account.hpp"
#include "model/queries/get_account_assets.hpp"
#include "model/queries/get_asset_info.hpp"
#include "model/queries/get_roles.hpp"
#include "model/queries/get_signatories.hpp"
#include "model/queries/get_transactions.hpp"
#include "model/sha3_hash.hpp"

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

    bool ModelCryptoProviderImpl::verify(const Query &query) const {
      return iroha::verify(iroha::hash(query).to_string(),
                           query.signature.pubkey,
                           query.signature.signature);
    }

    bool ModelCryptoProviderImpl::verify(const Block &block) const {
      return std::all_of(
          block.sigs.begin(), block.sigs.end(), [block](const Signature &sig) {
            return iroha::verify(
                iroha::hash(block).to_string(), sig.pubkey, sig.signature);
          });
    }

    void ModelCryptoProviderImpl::sign(Block &block) const {
      auto signature = iroha::sign(
          iroha::hash(block).to_string(), keypair_.pubkey, keypair_.privkey);

      block.sigs.emplace_back(signature, keypair_.pubkey);
    }

    void ModelCryptoProviderImpl::sign(Transaction &transaction) const {
      auto signature = iroha::sign(iroha::hash(transaction).to_string(),
                                   keypair_.pubkey,
                                   keypair_.privkey);

      transaction.signatures.emplace_back(signature, keypair_.pubkey);
    }

    void ModelCryptoProviderImpl::sign(Query &query) const {
      auto signature = iroha::sign(
          iroha::hash(query).to_string(), keypair_.pubkey, keypair_.privkey);

      query.signature = Signature{signature, keypair_.pubkey};
    }
  }  // namespace model
}  // namespace iroha

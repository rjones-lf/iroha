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

#include "crypto_provider/impl/crypto_provider_impl.hpp"

#include <algorithm>

#include "interfaces/iroha_internal/block.hpp"
#include "interfaces/queries/query.hpp"
#include "interfaces/transaction.hpp"

// TODO Alexey Chernyshov 2018-03-08 IR-968 - old model should be removed
// after relocation to shared_model
#include "cryptography/ed25519_sha3_impl/internal/ed25519_impl.hpp"
#include "model/block.hpp"
#include "model/query.hpp"
#include "model/sha3_hash.hpp"
#include "model/transaction.hpp"

namespace iroha {

  CryptoProviderImpl::CryptoProviderImpl(
      const shared_model::crypto::Keypair &keypair)
      : keypair_(keypair) {
    // TODO Alexey Chernyshov 2018-03-08 IR-968 - old model should be removed
    // after relocation to shared_model
    std::unique_ptr<iroha::keypair_t> old_key(keypair.makeOldModel());
    old_keypair_ = *old_key;
  }

  bool CryptoProviderImpl::verify(
      const shared_model::interface::Block &block) const {
    return true;
  }

  bool CryptoProviderImpl::verify(
      const shared_model::interface::Query &query) const {
    return true;
  }

  bool CryptoProviderImpl::verify(
      const shared_model::interface::Transaction &tx) const {
    return true;
  }

  void CryptoProviderImpl::sign(shared_model::interface::Block &block) const {}

  void CryptoProviderImpl::sign(shared_model::interface::Query &query) const {}

  void CryptoProviderImpl::sign(
      shared_model::interface::Transaction &transaction) const {}

  //-----------------------------------------------------------------------
  // TODO Alexey Chernyshov 2018-03-08 IR-968 - old model should be removed
  // after relocation to shared_model
  CryptoProviderImpl::CryptoProviderImpl(const keypair_t &keypair)
      : keypair_(shared_model::crypto::PublicKey(keypair.pubkey.to_string()),
                 shared_model::crypto::PrivateKey(keypair.privkey.to_string())),
        old_keypair_(keypair) {}

  bool CryptoProviderImpl::verify(const model::Transaction &tx) const {
    return std::all_of(tx.signatures.begin(),
                       tx.signatures.end(),
                       [tx](const model::Signature &sig) {
                         return iroha::verify(iroha::hash(tx).to_string(),
                                              sig.pubkey,
                                              sig.signature);
                       });
  }

  bool CryptoProviderImpl::verify(const model::Query &query) const {
    return iroha::verify(iroha::hash(query).to_string(),
                         query.signature.pubkey,
                         query.signature.signature);
  }

  bool CryptoProviderImpl::verify(const model::Block &block) const {
    return std::all_of(block.sigs.begin(),
                       block.sigs.end(),
                       [block](const model::Signature &sig) {
                         return iroha::verify(iroha::hash(block).to_string(),
                                              sig.pubkey,
                                              sig.signature);
                       });
  }

  void CryptoProviderImpl::sign(model::Block &block) const {
    auto signature = iroha::sign(iroha::hash(block).to_string(),
                                 old_keypair_.pubkey,
                                 old_keypair_.privkey);

    block.sigs.emplace_back(signature, old_keypair_.pubkey);
  }

  void CryptoProviderImpl::sign(model::Transaction &transaction) const {
    auto signature = iroha::sign(iroha::hash(transaction).to_string(),
                                 old_keypair_.pubkey,
                                 old_keypair_.privkey);

    transaction.signatures.emplace_back(signature, old_keypair_.pubkey);
  }

  void CryptoProviderImpl::sign(model::Query &query) const {
    auto signature = iroha::sign(iroha::hash(query).to_string(),
                                 old_keypair_.pubkey,
                                 old_keypair_.privkey);

    query.signature = model::Signature{signature, old_keypair_.pubkey};
  }

}  // namespace iroha

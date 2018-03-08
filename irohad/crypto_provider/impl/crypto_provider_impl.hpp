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
#include "cryptography/keypair.hpp"
#include "cryptography/crypto_provider/crypto_defaults.hpp"
#include "cryptography/crypto_provider/crypto_model_signer.hpp"


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

  template <typename Algorithm = shared_model::crypto::DefaultCryptoAlgorithmType>
  class CryptoProviderImpl : public CryptoProvider {
   public:
    explicit CryptoProviderImpl(
        const shared_model::crypto::Keypair &keypair);

    bool verify(const shared_model::interface::Block &block) const override;

    bool verify(const shared_model::interface::Query &query) const override;

    bool verify(const shared_model::interface::Transaction &tx) const override;

    void sign(shared_model::interface::Block &block) const override;

    void sign(shared_model::interface::Query &query) const override;

    void sign(shared_model::interface::Transaction &transaction) const override;

   private:
    shared_model::crypto::Keypair keypair_;

//    //---------------------------------------------------------------------
//    // TODO Alexey Chernyshov 2018-03-08 IR-968 - old model should be removed
//    // after relocation to shared_model
//   public:
//    explicit CryptoProviderImpl(const keypair_t &keypair);
//
//    virtual bool verify(const model::Block &block) const override;
//    virtual bool verify(const model::Query &query) const override;
//    virtual bool verify(const model::Transaction &tx) const override;
//    virtual void sign(model::Block &block) const override;
//    virtual void sign(model::Query &query) const override;
//    virtual void sign(model::Transaction &transaction) const override;

   private:
    keypair_t old_keypair_;

    shared_model::crypto::CryptoModelSigner<Algorithm> signer_;
  };

  template <typename Algorithm>
  CryptoProviderImpl<Algorithm>::CryptoProviderImpl(
      const shared_model::crypto::Keypair &keypair)
      : keypair_(keypair), signer_(keypair_) {
    // TODO Alexey Chernyshov 2018-03-08 IR-968 - old model should be removed
    // after relocation to shared_model
    std::unique_ptr<iroha::keypair_t> old_key(keypair.makeOldModel());
    old_keypair_ = *old_key;
  }

  template <typename Algorithm>
  bool CryptoProviderImpl<Algorithm>::verify(
      const shared_model::interface::Block &block) const {
    return block.signatures().size() > 0
        and std::all_of(
            block.signatures().begin(),
            block.signatures().end(),
            [this, &block](const shared_model::detail::PolymorphicWrapper<
                shared_model::interface::Signature> &signature) {
              return shared_model::crypto::CryptoVerifier<>::verify(
                  signature->signedData(),
                  block.payload(),
                  signature->publicKey());
            });
  }

  template <typename Algorithm>
  bool CryptoProviderImpl<Algorithm>::verify(
      const shared_model::interface::Query &query) const {
    return query.signatures().size() > 0
        and std::all_of(
            query.signatures().begin(),
            query.signatures().end(),
            [this, &query](const shared_model::detail::PolymorphicWrapper<
                shared_model::interface::Signature> &signature) {
              return shared_model::crypto::CryptoVerifier<>::verify(
                  signature->signedData(),
                  query.payload(),
                  signature->publicKey());
            });
  }

  template <typename Algorithm>
  bool CryptoProviderImpl<Algorithm>::verify(
      const shared_model::interface::Transaction &tx) const {
    return tx.signatures().size() > 0
        and std::all_of(
            tx.signatures().begin(),
            tx.signatures().end(),
            [this, &tx](const shared_model::detail::PolymorphicWrapper<
                shared_model::interface::Signature> &signature) {
              return shared_model::crypto::CryptoVerifier<>::verify(
                  signature->signedData(),
                  tx.payload(),
                  signature->publicKey());
            });
  }

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

//  //-----------------------------------------------------------------------
//  // TODO Alexey Chernyshov 2018-03-08 IR-968 - old model should be removed
//  // after relocation to shared_model
//  template <typename Algorithm>
//  CryptoProviderImpl<Algorithm>::CryptoProviderImpl(const keypair_t &keypair)
//      : keypair_(shared_model::crypto::PublicKey(keypair.pubkey.to_string()),
//                 shared_model::crypto::PrivateKey(keypair.privkey.to_string())),
//        old_keypair_(keypair),
//        signer_(keypair_) {}
//
//  template <typename Algorithm>
//  bool CryptoProviderImpl<Algorithm>::verify(
//      const model::Transaction &tx) const {
//    return std::all_of(tx.signatures.begin(),
//                       tx.signatures.end(),
//                       [tx](const model::Signature &sig) {
//                         return iroha::verify(iroha::hash(tx).to_string(),
//                                              sig.pubkey,
//                                              sig.signature);
//                       });
//  }
//
//  template <typename Algorithm>
//  bool CryptoProviderImpl<Algorithm>::verify(const model::Query &query) const {
//    return iroha::verify(iroha::hash(query).to_string(),
//                         query.signature.pubkey,
//                         query.signature.signature);
//  }
//
//  template <typename Algorithm>
//  bool CryptoProviderImpl<Algorithm>::verify(const model::Block &block) const {
//    return std::all_of(block.sigs.begin(),
//                       block.sigs.end(),
//                       [block](const model::Signature &sig) {
//                         return iroha::verify(iroha::hash(block).to_string(),
//                                              sig.pubkey,
//                                              sig.signature);
//                       });
//  }
//
//  template <typename Algorithm>
//  void CryptoProviderImpl<Algorithm>::sign(model::Block &block) const {
//    auto signature = iroha::sign(iroha::hash(block).to_string(),
//                                 old_keypair_.pubkey,
//                                 old_keypair_.privkey);
//
//    block.sigs.emplace_back(signature, old_keypair_.pubkey);
//  }
//
//  template <typename Algorithm>
//  void CryptoProviderImpl<Algorithm>::sign(
//      model::Transaction &transaction) const {
//    auto signature = iroha::sign(iroha::hash(transaction).to_string(),
//                                 old_keypair_.pubkey,
//                                 old_keypair_.privkey);
//
//    transaction.signatures.emplace_back(signature, old_keypair_.pubkey);
//  }
//
//  template <typename Algorithm>
//  void CryptoProviderImpl<Algorithm>::sign(model::Query &query) const {
//    auto signature = iroha::sign(iroha::hash(query).to_string(),
//                                 old_keypair_.pubkey,
//                                 old_keypair_.privkey);
//
//    query.signature = model::Signature{signature, old_keypair_.pubkey};
//  }

}  // namespace iroha

#endif  // IROHA_CRYPTO_PROVIDER_IMPL_HPP

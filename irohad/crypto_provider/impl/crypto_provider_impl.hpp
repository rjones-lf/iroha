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

#include "crypto_provider/crypto_provider.hpp"
#include "cryptography/keypair.hpp"

namespace iroha {

  class CryptoProviderImpl : public CryptoProvider {
   public:
    explicit CryptoProviderImpl(const shared_model::crypto::Keypair &keypair);

    bool verify(const shared_model::interface::Block &block) const override;

    bool verify(const shared_model::interface::Query &query) const override;

    bool verify(const shared_model::interface::Transaction &tx) const override;

    void sign(shared_model::interface::Block &block) const override;

    void sign(shared_model::interface::Query &query) const override;

    void sign(shared_model::interface::Transaction &transaction) const override;

   private:
    shared_model::crypto::Keypair keypair_;

    //---------------------------------------------------------------------
    // TODO Alexey Chernyshov 2018-03-08 IR-968 - old model should be removed
    // after relocation to shared_model
   public:
    explicit CryptoProviderImpl(const keypair_t &keypair);

    virtual bool verify(const model::Block &block) const override;
    virtual bool verify(const model::Query &query) const override;
    virtual bool verify(const model::Transaction &tx) const override;
    virtual void sign(model::Block &block) const override;
    virtual void sign(model::Query &query) const override;
    virtual void sign(model::Transaction &transaction) const override;

   private:
    keypair_t old_keypair_;
  };

}

#endif //IROHA_CRYPTO_PROVIDER_IMPL_HPP

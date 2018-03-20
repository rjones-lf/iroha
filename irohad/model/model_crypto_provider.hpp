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

#ifndef IROHA_MODEL_CRYPTO_PROVIDER_HPP
#define IROHA_MODEL_CRYPTO_PROVIDER_HPP

#include "model/block.hpp"
#include "model/query.hpp"
#include "model/transaction.hpp"

namespace iroha {
  namespace model {

    /**
     * [[deprecated]] Use irohad/crypto_provider with shared_model instead this
     * class. It is for compatibility with cli.
     * Crypto provider is an abstract service for making cryptography operations
     * for business logic objects (Model).
     */
    class ModelCryptoProvider {
     public:
      virtual ~ModelCryptoProvider() = default;

      /**
       * [[deprecated]] Use irohad/crypto_provider with shared_model instead this
       * class. It is for compatibility with cli.
       * Method for signature verification of a transaction.
       * @param tx - transaction for verification
       * @return true if transaction signature is valid, otherwise false
       */
      virtual bool verify(const Transaction &tx) const = 0;

      /**
       * [[deprecated]] Use irohad/crypto_provider with shared_model instead this
       * class. It is for compatibility with cli.
       * Method for signature verification of a query.
       * @param query - query for verification
       * @return true if query signature is valid, otherwise false
       */
      virtual bool verify(const Query &query) const = 0;

      /**
       * [[deprecated]] Use irohad/crypto_provider with shared_model instead this
       * class. It is for compatibility with cli.
       * Method for signature verification of a block.
       * @param block - block for verification
       * @return true if block signature is valid, otherwise false
       */
      virtual bool verify(const Block &block) const = 0;

      /**
       * [[deprecated]] Use irohad/crypto_provider with shared_model instead this
       * class. It is for compatibility with cli.
       * Method for signing a block with stored keypair
       * @param block - block for signing
       */
      virtual void sign(Block &block) const = 0;

      /**
       * [[deprecated]] Use irohad/crypto_provider with shared_model instead this
       * class. It is for compatibility with cli.
       * Method for signing a transaction with stored keypair
       * @param transaction - transaction for signing
       */
      virtual void sign(Transaction &transaction) const = 0;

      /**
       * [[deprecated]] Use irohad/crypto_provider with shared_model instead this
       * class. It is for compatibility with cli.
       * Method for signing a query with stored keypair
       * @param query - query to sign
       */
      virtual void sign(Query &query) const = 0;
    };
  }  // namespace model
}  // namespace iroha
#endif  // IROHA_MODEL_CRYPTO_PROVIDER_HPP

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

#ifndef IROHA_CRYPTO_PROVIDER_HPP
#define IROHA_CRYPTO_PROVIDER_HPP

#include "common/types.hpp"

namespace shared_model {
  namespace interface {
    class Block;
    class Query;
    class Transaction;
  }  // namespace interface
}  // namespace shared_model

namespace iroha {

  // TODO Alexey Chernyshov 2018-03-08 IR-968 - old model should be removed
  // after relocation to shared_model
  namespace model {
    struct Block;
    struct Query;
    struct Transaction;
  }  // namespace model

  /**
   * Crypto provider is an abstract service for making cryptography operations
   * for business logic objects (Model).
   */
  class CryptoProvider {
   public:
    virtual ~CryptoProvider() = default;

    /**
     * Method for signing a block with stored keypair
     * @param block - block for signing
     */
    virtual void sign(shared_model::interface::Block &block) const = 0;

    /**
     * Method for signing a query with stored keypair
     * @param query - query to sign
     */
    virtual void sign(shared_model::interface::Query &query) const = 0;

    /**
     * Method for signing a transaction with stored keypair
     * @param transaction - transaction for signing
     */
    virtual void sign(
        shared_model::interface::Transaction &transaction) const = 0;
  };

}  // namespace iroha
#endif  // IROHA_CRYPTO_PROVIDER_HPP

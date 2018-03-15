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

#ifndef IROHA_CRYPTO_VERIFIER_HPP
#define IROHA_CRYPTO_VERIFIER_HPP

#include "cryptography/blob.hpp"
#include "interfaces/common_objects/signable_hash.hpp"

namespace shared_model {
  namespace interface {
    class Block;
    class Query;
    class Transaction;
  }  // namespace interface
}  // namespace shared_model

namespace shared_model {
  namespace crypto {
    /**
     * CryptoVerifier - wrapper for generalization verification of cryptographic
     * signatures
     */
    class CryptoVerifier {
     public:
      virtual ~CryptoVerifier() = default;

      /**
       * Method for signature verification of a block.
       * @param block - block for verification
       * @return true if block signature is valid, otherwise false
       */
      virtual bool verify(
          const shared_model::interface::Block &block) const = 0;

      /**
       * Method for signature verification of a query.
       * @param query - query for verification
       * @return true if query signature is valid, otherwise false
       */
      virtual bool verify(
          const shared_model::interface::Query &query) const = 0;

      /**
       * Method for signature verification of a transaction.
       * @param tx - transaction for verification
       * @return true if transaction signature is valid, otherwise false
       */
      virtual bool verify(
          const shared_model::interface::Transaction &tx) const = 0;
    };

  }  // namespace crypto
}  // namespace shared_model

#endif  // IROHA_CRYPTO_VERIFIER_HPP
